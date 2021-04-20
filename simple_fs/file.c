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

static const struct spdk_file_operations simplefs_dir_ops = {
	.spdk_mkdir			= simple_dir_read,
	.spdk_readdir	 	= simple_dir_write,
	.spdk_writedir 		= simple_dir_open,
	.spdk_closedirte 	= simple_dir_create,
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
	assert(dir);
	struct spdkfs_dir *open_dir = dir;
	int io_unit_size = spdk_bs_get_io_unit_size(dir->fs);
	char *buffer = spdk_malloc(dir->dir_persist->d_dirent_count, io_unit_size, NULL, SPDK_ENV_SOCKET_ID_ANY,
				   SPDK_MALLOC_SHARE);

	generic_blob_io(dir->fs, dir,
			sizeof(struct spdkfs_file_persist_ctx), io_unit_size, buffer, true);

	int dirent_num = io_unit_size / sizeof(struct spdkfs_dirent);

}
// Always append to the bottom
void simple_dir_write(struct spdkfs_dir *dir)
{

}

void simple_dir_open(struct spdkfs_dir *dir)
{

	struct spdkfs_dir *open_dir = dir;

}
void simple_dir_create(struct spdkfs_dir *dir)
{
	struct spdkfs_dir* create_dir = dir;
	dir->_blob = blob;
	dir->
	if(cb_args->parent == -1) // Super blob, no need to add parent
	{
		return; 
	}
}
