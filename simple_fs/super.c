#include "spdkfs/fs.h"
#include "file.h"
#include "spdk/env.h"
#include "thread_poller.h"

struct spdk_filesystem *g_filesystem;
struct spdk_thread *g_spdkfs_thread;


struct spdk_filesystem* get_fs_instance()
{
	return g_filesystem;
}

static const char* JSON_CONFIG_FILE = "/usr/local/etc/spdk/rocksdb.json";
__attribute__((constructor)) void load_simple_spdk_fs();
__attribute__((destructor)) void unload_simple_spdk_fs();

static void load_fs_operations();

static void spdk_app_json_load_done(int rc, void *arg1)
{
	if(rc) 
	{
		SPDK_ERRLOG("Something wrong with spdk app json!\n");
	}
	bool* done = (bool*)arg1;
	*done = true;
}

static void bootstrap_fn(void* ctx) 
{
	spdk_app_json_config_load(JSON_CONFIG_FILE, SPDK_DEFAULT_RPC_ADDR, spdk_app_json_load_done,
				ctx, true);
}

static void spdk_init()
{
	struct spdk_env_opts opts;
	spdk_env_opts_init(&opts);
	if (spdk_env_init(&opts) < 0)
	{
		SPDK_ERRLOG("Unable to initialize SPDK env\n");
		exit(1);
	}
	spdk_thread_lib_init(NULL, 0);
	struct spdk_thread *spdk_init_thread = spdk_thread_create("init_thread", NULL);
	bool done = false;
	generic_poller(spdk_init_thread ,bootstrap_fn, &done, &done);
	if(!spdk_bdev_first()) {
        SPDK_ERRLOG("No bdev found, exit\n");
        exit(-1);
    }
}

static void load_root()
{
	uint64_t size = spdk_blob_get_num_clusters(g_filesystem->super_blob->blob);
	SPDK_WARNLOG("%d\n", size);
	if(size == 0) {
		SPDK_ERRLOG("Size = 0!\n");
	}
}

void load_simple_spdk_fs()
{
	spdk_init();
	g_spdkfs_thread =  spdk_thread_create("spdkfs_thread", NULL);
	spdk_set_thread(g_spdkfs_thread);
	struct spdk_fs_init_ctx ctx = {NULL, spdk_bdev_get_name(spdk_bdev_first()),  malloc(sizeof(bool))};
	*ctx.finished = false;
	generic_poller(g_spdkfs_thread, init_spdk_filesystem, &ctx, ctx.finished);
	g_filesystem = ctx.fs;
	SPDK_NOTICELOG("SPDK callback finished\n");
	spdk_blob_stat(&ctx);
	load_fs_operations();
	load_root();
}

void simple_fs_alloc_blob(struct spdk_filesystem *fs, spdk_fs_callback cb_fn,
			   void *cb_args);
void simple_fs_destroy_blob(struct spdk_blob *, spdk_fs_callback cb_fn, void *cb_args);
void simple_fs_free_blob(struct spdk_blob *, spdk_fs_callback cb_fn, void *cb_args);

static const struct spdk_fs_operations simple_fs_operations = {
	.alloc_blob		= simple_fs_alloc_blob,
	.destroy_blob	= simple_fs_destroy_blob,
	.free_blob		= simple_fs_free_blob,
};



static void load_fs_operations()
{
	g_filesystem->operations = malloc(sizeof(struct spdk_fs_operations));
	g_filesystem->operations = &simple_fs_operations;
}

static void open_blob_finished(void *cb_arg, struct spdk_blob *blb, int bserrno)
{
	struct simple_fs_cb_args *args = cb_arg;
	if (bserrno) {
		args->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		SPDK_ERRLOG("Open blob failed!\n");
		*args->done = true;
		return;
	}
	args->op_blob = blb;
	args->status = SIMPLE_OP_STATUS_SUCCCESS;
	if (args->cb_fn) {
		args->cb_fn(cb_arg);
	}
	*args->done = true;
	return;
}

static void create_blob_finished(void *cb_arg, spdk_blob_id blobid, int bserrno)
{
	struct simple_fs_cb_args *args = cb_arg;
	if (bserrno) {
		args->status = SIMPLE_OP_STATUS_NO_FREE_SPACE;
		SPDK_ERRLOG("Create blob failed!\n");
		*args->done = true;
		return;
	}
	spdk_bs_open_blob(g_filesystem->bs, blobid, open_blob_finished, cb_arg);
}

void simple_fs_alloc_blob(struct spdk_filesystem *fs, spdk_fs_callback cb_fn,
			   void *cb_args)
{

	struct simple_fs_cb_args *args = cb_args;
	args ->cb_fn = cb_fn;
	spdk_bs_create_blob(fs->bs, create_blob_finished,  cb_args);
}

static void delete_blob_finished(void *cb_arg, int bserrno)
{
	struct simple_fs_cb_args *args = cb_arg;
	if (bserrno) {
		args->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		SPDK_ERRLOG("Delete blob failed!\n");
	}
	*args->done = true;
	return;
}

void simple_fs_destroy_blob(struct spdk_blob *blob, spdk_fs_callback cb_fn, void *cb_args)
{
	spdk_bs_delete_blob(g_filesystem->bs, spdk_blob_get_id(blob), delete_blob_finished, cb_args);
}

static void close_blob_finished(void *cb_arg, int bserrno)
{
	struct simple_fs_cb_args *args = cb_arg;
	if (bserrno) {
		args->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		SPDK_ERRLOG("Close blob failed!\n");
	}
	*args->done = true;
	return;
}

void simple_fs_free_blob(struct spdk_blob *blob, spdk_fs_callback cb_fn, void *cb_args)
{
	struct simple_fs_cb_args *args = cb_args;
	spdk_blob_close(args->op_blob, close_blob_finished, cb_args);
}



static void unload_complete_cb(void *cb_arg, int bserrno)
{
	if(bserrno)
	{
		SPDK_ERRLOG("Unload blobstore failed!\n");
	}
	bool* done = cb_arg;
	*done = true;
}

static void unload_superblob_fn(void* ctx)
{
	spdk_blob_close(g_filesystem->super_blob->blob, unload_complete_cb, ctx);
}

static void unload_bs_fn(void* ctx)
{
	spdk_bs_free_io_channel(g_filesystem->io_channel);
	spdk_bs_unload(g_filesystem->bs, unload_complete_cb, ctx);
}

static void stop_subsystem_complete_cb(void* ctx)
{
	bool* done = ctx;
	*done = true;
}

static void stop_subsystem(void* ctx)
{
	spdk_subsystem_fini(stop_subsystem_complete_cb, ctx);
}

void unload_simple_spdk_fs()
{
	bool done = false;
	generic_poller(g_spdkfs_thread, unload_superblob_fn, &done, &done);
	done = false;
	generic_poller(g_spdkfs_thread, unload_bs_fn, &done, &done);
	done = false;
	generic_poller(g_spdkfs_thread, stop_subsystem_complete_cb, &done, &done);
	spdk_thread_exit(g_spdkfs_thread);
	done = false;
	generic_poller(g_spdkfs_thread, stop_subsystem_complete_cb, &done, &done);
	spdk_thread_destroy(g_spdkfs_thread);
	free(g_filesystem);
	spdk_env_dpdk_post_fini();
}