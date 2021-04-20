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
	.spdk_close = simple_fs_close,
};

static const struct spdk_dir_operations simplefs_dir_ops = {
	.spdk_mkdir			= simple_dir_create,
	.spdk_readdir	 	= simple_dir_read,
	.spdk_writedir 		= simple_dir_write,
	.spdk_closedir	 	= simple_dir_close,
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
	size_t len;
	spdk_blob_get_xattr_value(blob, "file_persistent", &file->file_persist, &len);
	assert(len == sizeof(struct spdkfs_file_persist_ctx));
}
void simple_fs_create(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{
	file->file_persist->f_size = 0;
	file->file_persist->_blob_id =  spdk_blob_get_id(file->_blob);
	file->file_persist->i_writecount = 0;
	file->file_persist->i_ctime = time(NULL);
	spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
			    sizeof(struct spdkfs_file_persist_ctx));

}
void simple_fs_release(struct spdk_blob *blob, struct spdkfs_file *file, void *cb_args)
{

}


void simple_fs_close(struct spdkfs_file *file, void *cb_args)
{
	spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
			sizeof(struct spdkfs_file_persist_ctx));
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
void simple_dir_read(struct spdkfs_dir *dir)
{
	assert(dir->fs);
	size_t len;
	spdk_blob_get_xattr_value(dir->blob, "dir_persistent", &dir->dir_persist,&len);
	assert(len ==  sizeof(struct spdkfs_dir_persist_ctx));
	int io_unit = spdk_bs_get_io_unit_size(dir->fs->bs);
	dir->dirents = spdk_malloc(dir->dir_persist->d_size, io_unit, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
	generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, true);

}
// Always append to the bottom
void simple_dir_write(struct spdkfs_dir *dir)
{
	if(!dir->dirty)
		return;
	dir->dirty = false;
	spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist, sizeof(struct spdkfs_dir_persist_ctx));
	generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, false);
}

void simple_dir_close(struct spdkfs_dir *dir)
{
	if(dir->dirty)
		generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, false);
	blob_close(dir->blob);
}
void simple_dir_create(struct spdkfs_dir *dir)
{
	// Persist data
	dir->dir_persist->d_dirent_count = 0;
	dir->dir_persist->_blob_id = spdk_blob_get_id(dir->blob);
	dir->dir_persist->i_ctime = time(NULL);
	spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist, sizeof(struct spdkfs_dir_persist_ctx));

	// In-memory data
	dir->dir_persist->d_size = 0;
	dir->initialized = true;
	dir->dirty = false;

}
