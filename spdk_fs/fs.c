#include "fs.h"

struct bs_load_context {
    struct spdk_filesystem* fs;
    bool is_loading;
    struct spdk_bs_bdev* bdev;
    bool* done;
};
static void
base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
		   void *event_ctx)
{
	SPDK_WARNLOG("Unsupported bdev event: type %d\n", type);
}

static void spdk_init_super_block_cb(void *cb_arg, struct spdk_blob_store *bs,
		int bserrno)
{
    struct bs_load_context* ctx = cb_arg;
    if(bserrno) { // Init failed
        SPDK_WARNLOG("Load failed, try initializing spdk fs\n");
        if(ctx->is_loading) { // It is possible that the blobstore is not initialized before
            ctx->is_loading = false;
            spdk_bs_init(ctx->bdev, NULL, spdk_init_super_block_cb, ctx);
        }
    }
    else { // Init success
        ctx->fs->bs = bs;
        ctx->fs->op_thread = spdk_get_thread();
        SPDK_NOTICELOG("Super block load success\n");
        *ctx->done = true;
    }
}

// Assume the thread lib is correctly set up
void init_spdk_filesystem(struct spdk_filesystem* fs, bool* done) {
    struct bs_load_context* ctx = malloc(sizeof(struct bs_load_context));
    ctx->fs = malloc(sizeof(struct spdk_filesystem));
	struct spdk_bs_bdev* bdev = NULL;

	spdk_bdev_create_bs_dev_ext(spdk_bdev_get_name(spdk_bdev_first()), base_bdev_event_cb, NULL, &bdev);
    
    ctx->is_loading = true;
    ctx->bdev = bdev;
    ctx->done = done;
    spdk_bs_load(ctx->bdev, NULL, spdk_init_super_block_cb, ctx);
    
}


static void cleanup_finished_cb(void *cb_arg, int bserrno) {
    if(bserrno) {
        SPDK_ERRLOG("Clean up failed!\n");
    }
}

void cleanup_filesystem(struct spdk_filesystem* fs) {
    spdk_bs_unload(fs->bs, cleanup_finished_cb, NULL);
    // TODO : fill in clean up
}

void spdk_blob_stat(struct spdk_filesystem* fs) {
   SPDK_NOTICELOG("spdk_bs_get_cluster_size %lu\n", spdk_bs_get_cluster_size(fs->bs));
   SPDK_NOTICELOG("spdk_bs_get_io_unit_size %lu\n", spdk_bs_get_io_unit_size(fs->bs));
   SPDK_NOTICELOG("spdk_bs_free_cluster_count %lu\n", spdk_bs_free_cluster_count(fs->bs)); 
}