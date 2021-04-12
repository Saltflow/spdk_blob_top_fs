#include "spdkfs/fs.h"
#include "file.h"

struct spdk_filesystem *g_filesystem;
struct spdk_thread *g_spdkfs_thread;


__attribute__((constructor)) void load_simple_spdk_fs();

void load_simple_spdk_fs()
{
	g_spdkfs_thread =  spdk_thread_create("spdkfs_thread", NULL);
	spdk_set_thread(g_spdkfs_thread);
	struct spdk_fs_context ctx;
	ctx.finished = malloc(sizeof(bool));
	*ctx.finished = false;
	spdk_thread_send_msg(g_spdkfs_thread, init_spdk_filesystem, &ctx);
	do {
		spdk_thread_poll(g_spdkfs_thread, 0, 0);
	} while (!*ctx.finished);
	g_filesystem = ctx.fs;
	SPDK_NOTICELOG("SPDK callback finished\n");
	spdk_blob_stat(&ctx);
	load_fs_operations();
}

void *simple_fs_alloc_blob(struct spdk_filesystem *fs, spdk_fs_callback cb_fn,
				       void *cb_args);
void simple_fs_destroy_blob(struct spdk_blob *, spdk_fs_callback cb_fn, void *cb_args);
void simple_fs_free_blob(struct spdk_blob *, spdk_fs_callback cb_fn, void *cb_args);

static const struct spdk_fs_operations simple_fs_operations = {
	.alloc_blob		= simple_fs_alloc_blob,
	.destroy_blob	= simple_fs_destroy_blob,
	.free_blob		= simple_fs_free_blob,
};



void load_fs_operations()
{
	g_filesystem->operations = malloc(sizeof(struct spdk_fs_operations));
	g_filesystem->operations = &simple_fs_operations;
}

static void open_blob_finished(void *cb_arg, struct spdk_blob *blb, int bserrno)
{
	struct simple_fs_cb_args* args = cb_arg;
	if(bserrno) {
		args->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		SPDK_ERRLOG("Open blob failed!\n");
		args->done = true;
		return;
	}
	args->op_blob = blb;
	args->status = SIMPLE_OP_STATUS_SUCCCESS;
	if(args->cb_fn)
	{
		args->cb_fn(cb_arg);
	}
	args->done = true;
	return;
}

static void create_blob_finished(void *cb_arg, spdk_blob_id blobid, int bserrno)
{
	struct simple_fs_cb_args *args = cb_arg;
	if(bserrno) {
		args->status = SIMPLE_OP_STATUS_NO_FREE_SPACE;
		SPDK_ERRLOG("Create blob failed!\n");
		args->done = true;
		return;
	}
	spdk_bs_open_blob(g_filesystem->bs, blobid, open_blob_finished, cb_arg);
}

void *simple_fs_alloc_blob(struct spdk_filesystem *fs, spdk_fs_callback cb_fn,
				       void *cb_args)
{
	
	struct simple_fs_cb_args *args = cb_args;
	args ->cb_fn = cb_fn;
	spdk_bs_create_blob(fs->bs, create_blob_finished,  cb_args);
}

static void delete_blob_finished(void *cb_arg, int bserrno) {
	struct simple_fs_cb_args *args = cb_arg;
	if(bserrno) {
		args->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		SPDK_ERRLOG("Delete blob failed!\n");
	}
	args->done = true;
	return;
}

void simple_fs_destroy_blob(struct spdk_blob* blob, spdk_fs_callback cb_fn, void *cb_args)
{
	spdk_bs_delete_blob(g_filesystem->bs, spdk_blob_get_id(blob), delete_blob_finished, cb_args);
}

static void close_blob_finished(void *cb_arg, int bserrno) {
	struct simple_fs_cb_args *args = cb_arg;
	if(bserrno) {
		args->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		SPDK_ERRLOG("Close blob failed!\n");
	}
	args->done = true;
	return;
}

void simple_fs_free_blob(struct spdk_blob *blob, spdk_fs_callback cb_fn, void *cb_args)
{
	struct simple_fs_cb_args *args = cb_args;
	spdk_blob_close(args->op_blob, close_blob_finished, cb_args);
}

