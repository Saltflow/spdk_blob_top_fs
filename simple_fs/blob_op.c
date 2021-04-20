#include"blob_op.h"
#include"thread_poller.h"
#include"file.h"

extern struct spdk_filesystem *g_filesystem;

struct blob_rw_ctx {
	bool *done;
	int blob_errno;
	struct spdk_filesystem *fs;
	struct spdk_blob *rw_blob;
	loff_t rw_offset;
	size_t rw_size;
	void *rw_buffer;
	bool read;
};


static void alloc_blob(void *ctx)
{
	g_filesystem->operations->alloc_blob(g_filesystem, ctx);
}

bool blob_create(struct spdk_blob **blob)
{
	bool done;
	struct fs_blob_ctx args = {&done, 0, *blob, 0};
	generic_poller(g_filesystem->op_thread, alloc_blob, &args, &done);
	*blob = args.op_blob;
	assert(*blob);
	SPDK_WARNLOG("blob %lu", spdk_blob_get_id(*blob));
	if (!args.fs_errno) {
		return true;
	} else {
		return false;
	}
}

static void open_blob_complete(void *cb_arg, struct spdk_blob *blb, int bserrno)
{
	struct fs_blob_ctx *args = cb_arg;
	args->fs_errno = bserrno;;
	if (bserrno) {
		SPDK_ERRLOG("Something wrong when open the blob! bserrno = %d\n", bserrno);
		args->done = true;
		return;
	}
	args->op_blob = blb;
	args->done = true;
}

static void open_blob(void *ctx)
{
	struct fs_blob_ctx *args = ctx;
	spdk_bs_open_blob(g_filesystem->bs, args->op_blob_id, open_blob_complete, ctx);
}

bool blob_open(struct spdk_blob **blob, spdk_blob_id blob_id)
{
	bool done;
	struct fs_blob_ctx args = {&done, 0, *blob, blob_id};
	generic_poller(g_filesystem->op_thread, open_blob, &args, &done);
	if (!args.fs_errno) {
		return true;
	} else {
		return false;
	}
}

static void close_blob_complete(void *cb_arg, int bserrno)
{
	struct fs_blob_ctx *args = cb_arg;
	args->fs_errno = bserrno;
	if (bserrno) {
		SPDK_ERRLOG("Something wrong when closing the blob! bserrno = %d\n", bserrno);
	}
	args->done = true;
}

static void close_blob(void *ctx)
{
	struct fs_blob_ctx *args = ctx;
	spdk_blob_close(args->op_blob, close_blob_complete, ctx);
}

bool blob_close(struct spdk_blob *blob)
{
	bool done;
	struct fs_blob_ctx args = {&done, 0, blob, 0};
	generic_poller(g_filesystem->op_thread, close_blob, &args, &done);
	if (!args.fs_errno) {
		return true;
	} else {
		return false;
	}
}


static void io_blob_complete(void *cb_arg, int bserrno)
{
	struct blob_rw_ctx *rw_ctx = cb_arg;
	rw_ctx->blob_errno = bserrno;
	if (bserrno) {
		SPDK_ERRLOG("IO Failed! bserrno %d, io type %s\n", bserrno, rw_ctx->read ? "Read" : "Write");
	}
	*rw_ctx->done = true;
}

static void io_blob(void *context)
{
	struct blob_rw_ctx *rw_ctx = context;
	uint64_t io_unit = spdk_bs_get_io_unit_size(rw_ctx->fs->bs);
	if (rw_ctx->read)
		spdk_blob_io_read(rw_ctx->rw_blob, rw_ctx->fs->io_channel, rw_ctx->rw_buffer,
				  rw_ctx->rw_offset / io_unit,
				  (rw_ctx->rw_size - 1) / io_unit + 1, io_blob_complete, rw_ctx);
	else
		spdk_blob_io_write(rw_ctx->rw_blob,  rw_ctx->fs->io_channel, rw_ctx->rw_buffer,
				   rw_ctx->rw_offset / io_unit,
				   (rw_ctx->rw_size - 1) / io_unit + 1, io_blob_complete, rw_ctx);
}


bool generic_blob_io(struct spdk_filesystem *fs, struct spdk_blob *blob, size_t size,
		     loff_t offset, void *buffer, bool read)
{
	bool done = false;
	struct blob_rw_ctx rw_ctx = {&done, 0, fs, blob, offset, size,  buffer, read};
	generic_poller(fs->op_thread, io_blob, &rw_ctx, &done);
	if (!rw_ctx.blob_errno) {
		return true;
	} else {
		return false;
	}
}

static void spdk_blob_sync_complete(void *cb_arg, int bserrno)
{
	struct blob_rw_ctx *rw_ctx = cb_arg;
	rw_ctx->blob_errno = bserrno;
	if (bserrno) {
		SPDK_ERRLOG("Sync Failed!\n");
	}
	SPDK_NOTICELOG("Resize blob success!\n");
	*rw_ctx->done = true;
}


static void resize_blob_complete(void *cb_arg, int bserrno)
{
	struct blob_rw_ctx *rw_ctx = cb_arg;
	rw_ctx->blob_errno = bserrno;
	if (bserrno) {
		SPDK_ERRLOG("Resize Failed!\n");
		*rw_ctx->done = true;
		return;
	}
	spdk_blob_sync_md(rw_ctx->rw_blob, spdk_blob_sync_complete, rw_ctx);
}

static void resize_blob(void *context)
{
	struct blob_rw_ctx *rw_ctx = context;
	uint64_t resize_unit = spdk_bs_get_cluster_size(rw_ctx->fs->bs);
	spdk_blob_resize(rw_ctx->rw_blob, (rw_ctx->rw_size - 1) / resize_unit + 1, resize_blob_complete,
			 rw_ctx);
}
// NOTE: resize are based on cluster ,not io_unit
bool generic_blob_resize(struct spdk_filesystem *fs, struct spdk_blob *blob, size_t size)
{
	bool done = false;
	struct blob_rw_ctx rw_ctx = {&done, NULL, fs, blob, NULL, size,  NULL, NULL};
	generic_poller(fs->op_thread, resize_blob, &rw_ctx, &done);
	if (rw_ctx.blob_errno) {
		return true;
	} else {
		return false;
	}
}
