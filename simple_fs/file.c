#include "spdkfs/fs.h"
#include "file.h"
#include "thread_poller.h"
#include "blob_op.h"

static const struct spdk_file_operations simple_fille_ops = {
	.spdk_lseek = simple_fs_lseek,
	.spdk_read = simple_fs_read,
	.spdk_write = simple_fs_write,
	.spdk_open = simple_fs_open,
	.spdk_create = simple_fs_create,
	.spdk_release = simple_fs_release,
};

static const struct spdk_file_operations simple_dir_ops = {
	.spdk_read = simple_dir_read,
	.spdk_write = simple_dir_write,
	.spdk_open = simple_dir_open,
	.spdk_create = simple_dir_create,
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


void bind_ops(struct spdkfs_dir *dir)
{
	dir->d_op = &simple_dir_ops;
}


void simple_dir_read(struct spdkfs_file *dir, size_t size, loff_t *buffer, void *ctx)
{
	struct spdkfs_dir *open_dir = dir;

}
void simple_dir_write(struct spdkfs_file *dir, size_t size, loff_t *buffer, void *ctx);

void simple_dir_open(struct spdk_blob *blob, struct spdkfs_file *dir, void *ctx)
{

	struct spdkfs_dir *open_dir = dir;
	open_dir->blob = blob;
	if (!ctx) {
		return;
	}
	struct simple_fs_dir_ctx *dir_ctx = ctx;
}
void simple_dir_create(struct spdk_blob *blob, struct spdkfs_file *dir, void *ctx)
{
	struct file_op_cb_args *cb_args = ctx;
	dir->_blob = blob;
	dir->fs = cb_args->fs;
	if (spdk_blob_get_id(dir->_blob) == spdk_blob_get_id(cb_args->fs->super_blob->blob)) {
		return;
	}
}

struct spdkfs_file *find_file(const char *__file, struct spdkfs_dir *dir)
{
	if (dir->dirent_count == 0) {
		return true;
	}
	for (int i = 0; i < dir->dirent_count; ++i) {

	}
}


struct spdkfs_file *spdkfs_create(const char *__file, struct spdkfs_dir *dir)
{
	struct spdkfs_file *new_file;
	// Find if there exist an identical file
	// If there exists, return the file
	// Otherwise, allocate a blob , then fill it with file meta.

}
