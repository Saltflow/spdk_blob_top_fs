#include "fs.h"

static struct spdk_filesystem* g_filesystem;

struct bs_load_context {
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
        g_filesystem->bs = bs;
        g_filesystem->op_thread = spdk_get_thread();
        SPDK_NOTICELOG("Super block load success\n");
        *ctx->done = true;
    }
}

// Assume 
void init_spdk_filesystem(bool* done) {
    g_filesystem = malloc(sizeof(struct spdk_filesystem));
	struct spdk_bs_bdev* bdev = NULL;

	spdk_bdev_create_bs_dev_ext(spdk_bdev_get_name(spdk_bdev_first()), base_bdev_event_cb, NULL, &bdev);
    
    struct bs_load_context* ctx = malloc(sizeof(struct bs_load_context));
    ctx->is_loading = true;
    ctx->bdev = bdev;
    ctx->done = done;
    spdk_bs_load(ctx->bdev, NULL, spdk_init_super_block_cb, ctx);
    
}

bool load_fs_ops(struct spdk_fs_operations *ops) {
    g_filesystem->operations = ops;
    return true;
}

void cleanup_filesystem() {
    // TODO : fill in clean up
}
