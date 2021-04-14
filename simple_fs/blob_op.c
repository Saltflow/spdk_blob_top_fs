#include"blob_op.h"
#include"thread_poller.h"
#include"file.h"

struct spdk_filesystem* now_fs;

static void alloc_blob_poller(void* ctx)
{
    now_fs->operations->alloc_blob(now_fs, NULL, ctx);
}

bool blob_create(struct spdk_blob *blob)
{
    struct simple_fs_cb_args arg;
    arg.done = malloc(sizeof(bool));
    now_fs = get_fs_instance();
    generic_poller(spdk_get_thread(), alloc_blob_poller, &arg, arg.done);
    if(arg.status)
    {
        SPDK_ERRLOG("blob create failed!\n");
        return false;
    }
    return true;
}

static struct spdk_fs_generic_ctx* rw_context_wrapper(struct spdk_filesystem* fs, struct spdk_blob* blob, 
    size_t size, loff_t offset, void* buffer, bool* done, bool read) {
    struct spdk_fs_rw_ctx* rw_ctx = malloc(sizeof(struct spdk_fs_rw_ctx));
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


static void io_blob_complete(void *cb_arg, int bserrno)
{
    struct spdk_fs_generic_ctx* ctx = cb_arg;
    struct spdk_fs_rw_ctx* rw_ctx = ctx->args;
    if(bserrno) {
        SPDK_ERRLOG("IO Failed! bserrno %d, io type %s\n", bserrno, rw_ctx->read? "Read" : "Write");
        rw_ctx->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
        *ctx->done = true;
        return;
    }
    *ctx->done = true;
    rw_ctx->status = SIMPLE_OP_STATUS_SUCCCESS;
}

static void io_blob(void* context)
{
    struct spdk_fs_generic_ctx* ctx = context;
    struct spdk_fs_rw_ctx* rw_ctx = ctx->args;
    if(rw_ctx->read)
        spdk_blob_io_read(rw_ctx->blob, ctx->fs->bs, rw_ctx->buffer, rw_ctx->offset, 
            rw_ctx->size, io_blob_complete, ctx);
    else
        spdk_blob_io_write(rw_ctx->blob, ctx->fs->bs, rw_ctx->buffer, rw_ctx->offset, 
            rw_ctx->size, io_blob_complete, ctx);
}


bool generic_blob_io(struct spdk_filesystem *fs,struct spdk_blob* blob, size_t size, loff_t *offset, void *buffer, bool read)
{
    bool done = false;
    struct spdk_fs_generic_ctx *ctx = rw_context_wrapper(fs, blob, size, offset, buffer, &done, read);
    struct spdk_fs_rw_ctx* rw_ctx = ctx->args;
    generic_poller(fs->op_thread, io_blob, ctx, &done);
    free(ctx);
    if(rw_ctx->status) {
        free(rw_ctx);
        return true;
    }
    else
    {
        free(rw_ctx);
        return false;
    }
}


static void resize_blob_complete(void *cb_arg, int bserrno)
{
    struct spdk_fs_generic_ctx* ctx = cb_arg;
    struct spdk_fs_rw_ctx* rw_ctx = ctx->args;
    if(bserrno) {
        SPDK_ERRLOG("Resize Failed!\n");
        rw_ctx->status = SIMPLE_OP_STATUS_UNKNOWN_FAILURE;
        *ctx->done = true;
        return;
    }
    rw_ctx->status = SIMPLE_OP_STATUS_SUCCCESS;
    *ctx->done = true;
}

static void resize_blob(void* context)
{
    struct spdk_fs_generic_ctx* ctx = context;
    struct spdk_fs_rw_ctx* rw_ctx = ctx->args;
    spdk_blob_resize(rw_ctx->blob, rw_ctx->size, resize_blob_complete, ctx);
}

bool generic_blob_resize(struct spdk_filesystem *fs, struct spdk_blob* blob, size_t size)
{   
    bool done = false;
    struct spdk_fs_generic_ctx* ctx = rw_context_wrapper(fs, blob, size, 0, NULL, &done);
    struct spdk_fs_rw_ctx* rw_ctx = ctx->args;
    generic_poller(fs->op_thread, resize_blob, ctx, &done);
    free(ctx);
    if(rw_ctx->status == SIMPLE_OP_STATUS_SUCCCESS) {
        free(rw_ctx);
        return true;
    }
    else
    {
        free(rw_ctx);
        return false;
    }
}