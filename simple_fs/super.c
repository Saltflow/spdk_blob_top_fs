// #include "spdk_fs/fs.h"

struct spdk_filesystem* g_filesystem;
struct spdk_thread* g_spdkfs_thread;


void load_simple_spdk_fs() {
    g_spdkfs_thread =  spdk_thread_create("spdkfs_thread", NULL);
    spdk_set_thread(g_spdkfs_thread);
    struct spdk_fs_context ctx;
    ctx.finished = malloc(sizeof(bool));
    *ctx.finished =false;
    spdk_thread_send_msg(g_spdkfs_thread, init_spdk_filesystem, &ctx);
    do {
        spdk_thread_poll(g_spdkfs_thread, 0, 0);
    }while (!*ctx.finished);
    g_filesystem = ctx.fs;
    SPDK_NOTICELOG("SPDK callback finished\n");
    spdk_blob_stat(&ctx);
}

struct spdk_blob *simple_fs_alloc_blob(struct spdk_filesystem *fs, spdk_fs_callback cb_fn, void* cb_args);
void simple_fs_destroy_blob(struct spdk_blob *, spdk_fs_callback cb_fn, void* cb_args);
void simple_fs_free_blob(struct spdk_blob *, spdk_fs_callback cb_fn, void* cb_args);

static const struct spdk_fs_operations simple_fs_operations = {
	.alloc_blob		= simple_fs_alloc_blob,
	.destroy_blob	= simple_fs_destroy_blob,
	.free_blob		= simple_fs_free_blob,
};

void load_fs_operations(){
    g_filesystem->operations = malloc(sizeof(struct spdk_fs_operations));
    g_filesystem->operations = &simple_fs_operations;
}

struct spdk_blob *simple_fs_alloc_blob(struct spdk_filesystem *fs, spdk_fs_callback cb_fn, void* cb_args) {
}

void simple_fs_destroy_blob(struct spdk_blob *blob, spdk_fs_callback cb_fn, void* cb_args) {

}
void simple_fs_free_blob(struct spdk_blob *blob, spdk_fs_callback cb_fn, void* cb_args) {

}