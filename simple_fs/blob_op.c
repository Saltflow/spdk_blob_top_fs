#include"blob_op.h"
#include"thread_poller.h"
#include"file.h"

struct spdk_filesystem* now_fs;

void alloc_blob_poller(void* ctx)
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


bool blob_open(struct spdk_blob* blob, int spdk_blob_id)
{
    struct simple_fs_cb_args arg;
    arg.done = malloc(sizeof(bool));
    now_fs = get_fs_instance();
}