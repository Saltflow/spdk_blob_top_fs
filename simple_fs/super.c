#include "spdkfs/fs.h"
#include "spdkfs/io_mm.h"
#include "file.h"
#include "spdk/env.h"
#include "thread_poller.h"
#include "blob_op.h"
struct spdk_filesystem *g_filesystem = NULL;
struct spdk_thread *g_spdkfs_thread;
extern struct spdkfs_dir *g_workdir;
static int META_SIZE = 0;

extern void initialize_interface();

static const char *JSON_CONFIG_FILE = "/usr/local/etc/spdk/rocksdb.json";
__attribute__((constructor)) void load_simple_spdk_fs();
__attribute__((destructor)) void unload_simple_spdk_fs();

static void load_fs_operations();

static void spdk_app_json_load_done(int rc, void *arg1)
{
	if (rc) {
		SPDK_ERRLOG("Something wrong with spdk app json!\n");
	}
	bool *done = (bool *)arg1;
	*done = true;
}

static void bootstrap_fn(void *ctx)
{
	spdk_app_json_config_load(JSON_CONFIG_FILE, SPDK_DEFAULT_RPC_ADDR, spdk_app_json_load_done,
				  ctx, true);
}

static void spdk_init()
{
	struct spdk_env_opts opts;
	spdk_env_opts_init(&opts);
	if (spdk_env_init(&opts) < 0) {
		SPDK_ERRLOG("Unable to initialize SPDK env\n");
		exit(1);
	}
	spdk_thread_lib_init(NULL, 0);
	struct spdk_thread *spdk_init_thread = spdk_thread_create("init_thread", NULL);
	bool done = false;
	generic_poller(spdk_init_thread, bootstrap_fn, &done, &done);
	if (!spdk_bdev_first()) {
		SPDK_ERRLOG("No bdev found, exit\n");
		exit(-1);
	}
}

static void load_root()
{
	g_filesystem->super_blob->root = malloc(sizeof(struct spdkfs_dir));
	bind_dir_ops(g_filesystem->super_blob->root);
	struct spdkfs_dir *root = g_filesystem->super_blob->root;
	root->blob = g_filesystem->super_blob->blob;
	root->fs = g_filesystem;
	uint64_t size = spdk_blob_get_num_clusters(g_filesystem->super_blob->blob);
	if (size == 0) { // There is no data in the super blob before
		SPDK_ERRLOG("Size = 0!\n");
		root->d_op->spdk_mkdir(root);
	} else {
		root->d_op->spdk_readdir(root);
	}
	root->parent = -1;
	g_workdir = root;
}

void load_simple_spdk_fs()
{
	initialize_interface();
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
	spdkfs_mm_init(g_filesystem);
	g_filesystem->_buffer.buffer = spdk_malloc(0x100000, spdk_bs_get_io_unit_size(g_filesystem->bs),
			NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
	g_filesystem->_buffer.buf_size = 0x100000;
}

void simple_fs_alloc_blob(struct spdk_filesystem *fs, struct fs_blob_ctx *cb_args);
void simple_fs_free_blob(struct spdk_blob *op_blob, struct fs_blob_ctx *cb_args);

static const struct spdk_fs_operations simple_fs_operations = {
	.alloc_blob		= simple_fs_alloc_blob,
	.free_blob		= simple_fs_free_blob,
};

static void load_fs_operations()
{
	g_filesystem->operations = malloc(sizeof(struct spdk_fs_operations));
	g_filesystem->operations = &simple_fs_operations;
}

static void open_blob_finished(void *cb_arg, struct spdk_blob *blb, int bserrno)
{
	struct fs_blob_ctx *args = cb_arg;
	if (bserrno) {
		args->fs_errno = bserrno;
		SPDK_ERRLOG("Open blob failed!\n");
		*args->done = true;
		return;
	}
	args->op_blob = blb;
	args->fs_errno = bserrno;
	*args->done = true;
	return;
}

static void create_blob_finished(void *cb_arg, spdk_blob_id blobid, int bserrno)
{
	struct fs_blob_ctx *args = cb_arg;
	if (bserrno) {
		args->fs_errno = bserrno;
		SPDK_ERRLOG("Create blob failed!\n");
		*args->done = true;
		return;
	}
	spdk_bs_open_blob(g_filesystem->bs, blobid, open_blob_finished, cb_arg);
}

void simple_fs_alloc_blob(struct spdk_filesystem *fs, struct fs_blob_ctx *cb_args)
{
	spdk_bs_create_blob(fs->bs, create_blob_finished,  cb_args);
}

static void delete_blob_finished(void *cb_arg, int bserrno)
{
	struct fs_blob_ctx *args = cb_arg;
	if (bserrno) {
		args->fs_errno = bserrno;
		SPDK_ERRLOG("Delete blob failed!\n");
	}
	*args->done = true;
	return;
}

void simple_fs_free_blob(struct spdk_blob *op_blob, struct fs_blob_ctx *cb_args)
{
	spdk_bs_delete_blob(g_filesystem->bs, spdk_blob_get_id(op_blob), delete_blob_finished, cb_args);
}

static void unload_complete_cb(void *cb_arg, int bserrno)
{
	if (bserrno) {
		SPDK_ERRLOG("Unload blobstore failed!\n");
	}
	bool *done = cb_arg;
	*done = true;
}


static void unload_bs_fn(void *ctx)
{
	spdk_bs_free_io_channel(g_filesystem->io_channel);
	spdk_bs_unload(g_filesystem->bs, unload_complete_cb, ctx);
}

static void stop_subsystem_complete_cb(void *ctx)
{
	bool *done = ctx;
	*done = true;
}

static void stop_subsystem(void *ctx)
{
	spdk_subsystem_fini(stop_subsystem_complete_cb, ctx);
}

void unload_simple_spdk_fs()
{
	bool done = false;
	spdkfs_mm_free();
	g_filesystem->super_blob->root->d_op->spdk_closedir(g_filesystem->super_blob->root);
	done = false;
	generic_poller(g_spdkfs_thread, unload_bs_fn, &done, &done);
	done = false;
	generic_poller(g_spdkfs_thread, stop_subsystem, &done, &done);
	free(g_filesystem);
	spdk_env_dpdk_post_fini();
}
