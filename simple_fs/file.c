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
void simple_fs_read(struct spdkfs_file *file, size_t size, void *buffer, void *cb_args)
{

}
void simple_fs_write(struct spdkfs_file *file, size_t size, void *buffer, void *cb_args)
{

}
void simple_fs_open(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{
	file->_blob = blob;
	struct file_persistent_ctx *file_persistent;
	spdk_blob_get_xattr_value(blob, "file_persistent", &file_persistent,
				  sizeof(struct spdkfs_file_persist_ctx));
	file->file_persist = file_persistent;


}
void simple_fs_create(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{
	file->_blob = blob;
	file->file_persist->f_size = 0;
	file->file_persist->i_parent_blob_id =  spdk_blob_get_id(file->_blob);
	file->file_persist->i_writecount = 0;
	file->file_persist->i_ctime = time(NULL);
	spdk_blob_set_xattr(blob, "file_persistent", file->file_persist,
			    sizeof(struct spdkfs_file_persist_ctx));

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
	assert(dir);
	assert(ctx);

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
	dir->f_op->spdk_read()
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
