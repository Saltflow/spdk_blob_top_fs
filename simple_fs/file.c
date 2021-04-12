#include "spdkfs/fs.h"
#include "file.h"

static const struct spdk_file_operations simple_ops = {
	.spdk_lseek = simple_fs_lseek,
	.spdk_read = simple_fs_read,
	.spdk_write = simple_fs_write,
	.spdk_open = simple_fs_open,
	.spdk_create = simple_fs_create,
	.spdk_release = simple_fs_release,
};


void simple_fs_lseek(struct spdkfs_file *file, loff_t offset, int f_flag, void *cb_args)
{

}
void simple_fs_read(struct spdkfs_file *file, size_t size, loff_t *buffer, void *cb_args)
{

}
void simple_fs_write(struct spdkfs_file *file, size_t size, loff_t *buffer, void *cb_args)
{

}
void simple_fs_open(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{
	struct file_op_cb_args *args = cb_args;
	file->_blob = blob;
	file->file_persist-> f_size = spdk_blob_get_num_pages(blob) * spdk_bs_get_page_size(args->fs);
	file->f_pos = 0;
}
void simple_fs_create(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{
	file->_blob = blob;
}
void simple_fs_release(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{

}


void simple_dir_read(struct spdkfs_file *dir, size_t size, loff_t *buffer, void *ctx);
void simple_dir_write(struct spdkfs_file *dir, size_t size, loff_t *buffer, void *ctx);
void simple_dir_open(struct spdk_blob *blob, struct spdkfs_file *dir, void *ctx);
void simple_dir_create(struct spdk_blob *blob, struct spdkfs_file *dir, void *ctx)
{
}