#include "fs.h"

struct bs_load_context {
	struct spdk_filesystem *fs;
	bool is_loading;
	struct spdk_bs_bdev *bdev;
	bool *done;
};
static void
base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
		   void *event_ctx)
{
	SPDK_WARNLOG("Unsupported bdev event: type %d\n", type);
}

static void spdk_bs_sb_open_complete(void *cb_arg, struct spdk_blob *blb, int bserrno)
{
	struct bs_load_context *ctx = cb_arg;
	if (bserrno) { // Init failed
		SPDK_WARNLOG("Super blob open failed, bserrno %d\n", bserrno);
		*ctx->done = true;
		return;
	}
	ctx->fs->super_blob = malloc(sizeof(struct spdk_super_blob));
	ctx->fs->super_blob->blob = blb;
	*ctx->done = true;
}

static void spdk_bs_create_super_complete(void *cb_arg, spdk_blob_id blobid, int bserrno);
static void spdk_bs_set_super_complete(void *cb_arg, int bserrno);

static void spdk_bs_get_super_complete(void *cb_arg, spdk_blob_id blobid, int bserrno)
{

	struct bs_load_context *ctx = cb_arg;
	if (bserrno) {
		SPDK_ERRLOG("Super blob get failed, bserrno %d\n", bserrno);
		if (bserrno == -ENOENT) {
			spdk_bs_create_blob(ctx->fs->bs, spdk_bs_create_super_complete, ctx);
		}
	} else {
		spdk_bs_open_blob(ctx->fs->bs, blobid, spdk_bs_sb_open_complete, ctx);
	}
}

static void spdk_bs_set_super_complete(void *cb_arg, int bserrno)
{
	struct bs_load_context *ctx = cb_arg;
	if (bserrno) {
		SPDK_ERRLOG("Set super failed, bserrno %d\n", bserrno);
		*ctx->done = true;
		return;
	}
}

static void spdk_bs_create_super_complete(void *cb_arg, spdk_blob_id blobid, int bserrno)
{
	struct bs_load_context *ctx = cb_arg;
	if (bserrno) {
		SPDK_ERRLOG("Super blob create failed, bserrno %d\n", bserrno);
		*ctx->done = true;
		return;
	} else {
		SPDK_WARNLOG("Blobid %lu\n", blobid);

		spdk_bs_set_super(ctx->fs->bs, blobid, spdk_bs_set_super_complete, ctx);
	}
}

static void spdk_init_super_block_cb(void *cb_arg, struct spdk_blob_store *bs,
				     int bserrno)
{
	struct bs_load_context *ctx = cb_arg;
	if (bserrno) { // Init failed
		SPDK_WARNLOG("Load failed, try initializing spdk fs\n");
		if (ctx->is_loading) { // It is possible that the blobstore is not initialized before
			ctx->is_loading = false;
			spdk_bs_init(ctx->bdev, NULL, spdk_init_super_block_cb, ctx);
			return;
		} else {
			SPDK_ERRLOG("Init failed, bserrno %d\n", bserrno);
		}
	} else { // Init success
		ctx->fs->bs = bs;
		ctx->fs->op_thread = spdk_get_thread();
		ctx->fs->io_channel = spdk_bs_alloc_io_channel(ctx->fs->bs);
		if (!ctx->fs->io_channel) {
			SPDK_ERRLOG("Error allocating io channel!\n");
		}
		if (ctx->is_loading) {
			SPDK_NOTICELOG("Super block successfully loaded\n");
			spdk_bs_get_super(ctx->fs->bs, spdk_bs_get_super_complete, ctx);
		} else {
			SPDK_NOTICELOG("Super block successfully initialized\n");
			// spdk_bs_get_super(ctx->fs->bs, spdk_bs_get_super_complete, ctx);
			// spdk_bs_get_super(ctx->fs->bs, spdk_bs_get_super_complete, ctx);
		}
	}
}

// Assume the thread lib is correctly set up
void init_spdk_filesystem(struct spdk_fs_init_ctx *fs_ctx)
{
	struct bs_load_context *ctx = malloc(sizeof(struct bs_load_context));
	ctx->fs = malloc(sizeof(struct spdk_filesystem));
	fs_ctx->fs = ctx->fs;
	struct spdk_bs_bdev *bdev = NULL;
	assert(fs_ctx->spdk_bdev_name);

	int rc = spdk_bdev_create_bs_dev_ext(fs_ctx->spdk_bdev_name, base_bdev_event_cb, NULL, &bdev);
	if (rc != 0) {
		SPDK_ERRLOG("Spdk bdev create bs failed\n");
		exit(-1);
	}
	ctx->is_loading = true;
	ctx->bdev = bdev;
	ctx->done = fs_ctx->finished;
	spdk_bs_load(ctx->bdev, NULL, spdk_init_super_block_cb, ctx);

}


static void cleanup_finished_cb(void *cb_arg, int bserrno)
{
	if (bserrno) {
		SPDK_ERRLOG("Clean up failed, bserrno %d\n", bserrno);
	}
	bool *finished = cb_arg;
	*finished = true;
}

void cleanup_filesystem(struct spdk_fs_init_ctx *ctx)
{
	spdk_bs_free_io_channel(ctx->fs->io_channel);
	spdk_bs_unload(ctx->fs->bs, cleanup_finished_cb, ctx->finished);
	// TODO : fill in clean up
}

void spdk_blob_stat(struct spdk_fs_init_ctx *ctx)
{
	SPDK_NOTICELOG("spdk_bs_get_cluster_size %lu\n", spdk_bs_get_cluster_size(ctx->fs->bs));
	SPDK_NOTICELOG("spdk_bs_get_io_unit_size %lu\n", spdk_bs_get_io_unit_size(ctx->fs->bs));
	SPDK_NOTICELOG("spdk_bs_free_cluster_count %lu\n", spdk_bs_free_cluster_count(ctx->fs->bs));
}
