#include "spdkfs/fs.h"
#include "file.h"
#include "thread_poller.h"
#include "blob_op.h"

static const struct spdk_file_operations simplefs_file_ops = {
	.spdk_lseek = simple_fs_lseek,
	.spdk_read = simple_fs_read,
	.spdk_write = simple_fs_write,
	.spdk_open = simple_fs_open,
	.spdk_create = simple_fs_create,
	.spdk_release = simple_fs_release,
};

static const struct spdk_file_operations simplefs_dir_ops = {
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
	struct general_op_cb_args *args = cb_args;
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


void bind_dir_ops(struct spdkfs_dir *dir)
{
	dir->d_op = &simplefs_dir_ops;
}

void bind_file_ops(struct spdkfs_file *file)
{
	file->f_op = &simplefs_file_ops;
}

// This function is expected to read once in the directory
void simple_dir_read(struct spdkfs_file *dir, size_t size, loff_t *buffer, void *ctx)
{
	struct spdkfs_dir *open_dir = dir;
	struct simple_fs_dir_ctx *dir_ctx = ctx;
	int io_unit_size = spdk_bs_get_io_unit_size(dir_ctx->_op.fs->bs);
	char *buffer = spdk_malloc(size, io_unit_size, NULL, SPDK_ENV_SOCKET_ID_ANY,
					SPDK_MALLOC_SHARE);

	generic_blob_io(dir_ctx->_op.fs, dir,
			sizeof(struct spdkfs_file_persist_ctx), io_unit_size, buffer, true);

	int dirent_num = size / sizeof(struct spdkfs_dirent_persist_ctx);

}
// Always append to the bottom
void simple_dir_write(struct spdkfs_file *dir, size_t size, loff_t *buffer, void *ctx)
{
	struct spdkfs_dir *open_dir = dir;

}

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
	struct general_op_cb_args *cb_args = ctx;
	dir->_blob = blob;
	dir->fs = cb_args->fs;
	if (spdk_blob_get_id(dir->_blob) == spdk_blob_get_id(cb_args->fs->super_blob->blob)) {
		return;
	}
}


struct spdkfs_file *spdkfs_create(const char *__file, struct spdkfs_dir *dir)
{
	assert(dir->initialized);
	struct spdkfs_file *new_file = malloc(sizeof(struct spdkfs_file));
	// Find if there exist an identical file
	// If there exists, return the file
	// Otherwise, allocate a blob , then sync the blob in the directory
	bind_file_ops(new_file);
	if (dir->dirent_count == 0) {
		blob_create(&new_file->_blob);
		dir->dirent_count++;
		struct spdkfs_dirent* new_dir = spdk_malloc(sizeof(struct spdkfs_dirent), 0, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
		dir->d_op->spdk_write(dir, sizeof(struct spdkfs_dirent), new_dir, NULL);
	}
	for (int i = 0; i < dir->dirent_count; ++i) {
		if(!strcmp(__file, dir->dirents[i].d_ctx._name))
		{
			return NULL;
		}
	}

}
