#include"blob_op.h"
#include"thread_poller.h"
#include"file.h"

extern struct spdk_filesystem *g_filesystem;


static struct spdk_fs_generic_ctx *rw_context_wrapper(struct spdk_filesystem *fs,
		struct spdk_blob *blob,
		size_t size, loff_t offset, void *buffer, bool *done, bool read)
{
	struct spdk_fs_rw_ctx *rw_ctx = malloc(sizeof(struct spdk_fs_rw_ctx));
	struct spdk_fs_generic_ctx *ctx = malloc(sizeof(struct spdk_fs_generic_ctx));
	ctx->args = rw_ctx;
	ctx->done = done;
	ctx->fs = fs;
	rw_ctx->blob = blob;
	rw_ctx->buffer = buffer;
	rw_ctx->offset = offset;
	rw_ctx->size = size;
	rw_ctx->read = read;
	return ctx;
}

static void alloc_blob(void *ctx)
{
	struct simple_fs_cb_args* args = ctx;
	g_filesystem->operations->alloc_blob(g_filesystem, NULL, args);
}

bool blob_create(struct spdk_blob **blob)
{
	enum simple_op_status status;
	bool done;
	struct simple_fs_cb_args args = {&done ,status, *blob, NULL};
	generic_poller(g_filesystem->op_thread, alloc_blob, &args, &done);
}


static void io_blob_complete(void *cb_arg, int bserrno)
{
	struct spdk_fs_generic_ctx *ctx = cb_arg;
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
	if (bserrno) {
		SPDK_ERRLOG("IO Failed! bserrno %d, io type %s\n", bserrno, rw_ctx->read ? "Read" : "Write");
		rw_ctx->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		*ctx->done = true;
		return;
	}
	*ctx->done = true;
	rw_ctx->status = SIMPLE_OP_STATUS_SUCCCESS;
}

static void io_blob(void *context)
{
	struct spdk_fs_generic_ctx *ctx = context;
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
    assert(rw_ctx->blob);
    assert(ctx->fs->bs);
    uint64_t io_unit = spdk_bs_get_io_unit_size(ctx->fs->bs);
	if (rw_ctx->read)
		spdk_blob_io_read(rw_ctx->blob, ctx->fs->bs, rw_ctx->buffer, rw_ctx->offset,
				  rw_ctx->size / io_unit, io_blob_complete, ctx);
	else
		spdk_blob_io_write(rw_ctx->blob, ctx->fs->bs, rw_ctx->buffer, rw_ctx->offset,
				   rw_ctx->size / io_unit, io_blob_complete, ctx);
}


bool generic_blob_io(struct spdk_filesystem *fs, struct spdk_blob *blob, size_t size,
		     loff_t *offset, void *buffer, bool read)
{
	bool done = false;
	struct spdk_fs_generic_ctx *ctx = rw_context_wrapper(fs, blob, size, offset, buffer, &done, read);
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
	generic_poller(fs->op_thread, io_blob, ctx, &done);
	free(ctx);
	if (rw_ctx->status) {
		free(rw_ctx);
		return true;
	} else {
		free(rw_ctx);
		return false;
	}
}

static void spdk_blob_sync_complete(void *cb_arg, int bserrno)
{
	struct spdk_fs_generic_ctx *ctx = cb_arg;
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
	if (bserrno) {
		SPDK_ERRLOG("Sync Failed!\n");
		rw_ctx->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		*ctx->done = true;
		return;
	}
    SPDK_NOTICELOG("Resize blob success!\n");
	rw_ctx->status = SIMPLE_OP_STATUS_SUCCCESS;
	*ctx->done = true;
}


static void resize_blob_complete(void *cb_arg, int bserrno)
{
	struct spdk_fs_generic_ctx *ctx = cb_arg;
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
	if (bserrno) {
		SPDK_ERRLOG("Resize Failed!\n");
		rw_ctx->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
		*ctx->done = true;
		return;
	}
    spdk_blob_sync_md(rw_ctx->blob, spdk_blob_sync_complete, ctx);
}

static void resize_blob(void *context)
{
	struct spdk_fs_generic_ctx *ctx = context;
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
    
    uint64_t io_unit = spdk_bs_get_io_unit_size(ctx->fs->bs);
	spdk_blob_resize(rw_ctx->blob, rw_ctx->size / io_unit, resize_blob_complete, ctx);
}

bool generic_blob_resize(struct spdk_filesystem *fs, struct spdk_blob *blob, size_t size)
{
	bool done = false;
	struct spdk_fs_generic_ctx *ctx = rw_context_wrapper(fs, blob, size, 0, NULL, &done, false);
	struct spdk_fs_rw_ctx *rw_ctx = ctx->args;
	generic_poller(fs->op_thread, resize_blob, ctx, &done);
	free(ctx);
	if (rw_ctx->status == SIMPLE_OP_STATUS_SUCCCESS) {
		free(rw_ctx);
		return true;
	} else {
		free(rw_ctx);
		return false;
	}
}
