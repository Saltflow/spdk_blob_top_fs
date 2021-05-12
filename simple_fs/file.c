#include "spdkfs/fs.h"
#include "file.h"
#include "thread_poller.h"
#include "blob_op.h"

static const struct spdk_file_operations simplefs_file_ops = {
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

static inline size_t get_blob_size(struct spdk_blob* blob, struct spdk_blob_store * bs)
{
	return spdk_blob_get_num_clusters(blob) * spdk_bs_get_cluster_size(bs);
}


ssize_t simple_fs_read(struct spdkfs_file *file, size_t size, void *buffer)
{
	assert(size % 512 == 0);
	if (file->f_pos + size > file->file_persist->f_size) {
		return;
	}
	generic_blob_io(file->fs, file->_blob, size, file->f_pos, buffer, true);
	file->f_pos += size;
	spdk_set_thread(file->fs->op_thread);
	spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
			    sizeof(struct spdkfs_file_persist_ctx));
}
ssize_t simple_fs_write(struct spdkfs_file *file, size_t size, void *buffer)
{
	size_t file_max_size = get_blob_size(file->_blob, file->fs->bs);
	if (file->f_pos + size > file_max_size) {
		generic_blob_resize(file->fs, file->_blob, file_max_size + spdk_bs_get_cluster_size(file->fs->bs));
	}
	if(file->f_pos > file->file_persist->f_size)
		file->file_persist->f_size = file->f_pos;
	file->file_persist->i_writecount++;
	generic_blob_io(file->fs, file->_blob, size, file->f_pos, buffer, false);
	file->f_pos += size;
	spdk_set_thread(file->fs->op_thread);
	spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
			    sizeof(struct spdkfs_file_persist_ctx));
}
void simple_fs_open(struct spdkfs_file *file)
{
	size_t len;
	spdk_set_thread(file->fs->op_thread);
	spdk_blob_get_xattr_value(file->_blob, "file_persistent", &file->file_persist, &len);
	assert(len == sizeof(struct spdkfs_file_persist_ctx));
}
void simple_fs_create(struct spdkfs_file *file)
{
	file->file_persist = malloc(sizeof(struct spdkfs_file_persist_ctx));
	file->file_persist->f_size = 0;
	file->file_persist->_blob_id =  spdk_blob_get_id(file->_blob);
	file->file_persist->i_writecount = 0;
	file->file_persist->i_ctime = time(NULL);
	spdk_set_thread(file->fs->op_thread);
	spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
			    sizeof(struct spdkfs_file_persist_ctx));

}
void simple_fs_release(struct spdkfs_file *file)
{

}


void simple_fs_close(struct spdkfs_file *file)
{
	spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
			    sizeof(struct spdkfs_file_persist_ctx));
	blob_close(file->_blob);
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
	spdk_blob_get_xattr_value(dir->blob, "dir_persistent", &dir->dir_persist, &len);
	assert(len ==  sizeof(struct spdkfs_dir_persist_ctx));
	int io_unit = spdk_bs_get_io_unit_size(dir->fs->bs);
	dir->dirents = spdk_malloc(dir->dir_persist->d_size, io_unit, NULL, SPDK_ENV_SOCKET_ID_ANY,
				   SPDK_MALLOC_SHARE);
	generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, true);

	// In-memory data
	dir->initialized = true;
	dir->dirty = false;
}
// Always append to the bottom
void simple_dir_write(struct spdkfs_dir *dir)
{
	if (!dir->dirty) {
		return;
	}
	dir->dirty = false;
	spdk_set_thread(dir->fs->op_thread);
	spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist,
			    sizeof(struct spdkfs_dir_persist_ctx));
	generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, false);
}

void simple_dir_close(struct spdkfs_dir *dir)
{
	if (dir->dirty) {
		if(dir->dir_persist->d_size > get_blob_size(dir->blob, dir->fs->bs))
			generic_blob_resize(dir->fs, dir->blob, dir->dir_persist->d_size);
		generic_blob_io(dir->fs, dir->blob, dir->dir_persist->d_size, 0, dir->dirents, false);
		spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist, sizeof(struct spdkfs_dir_persist_ctx));
	}
	blob_close(dir->blob);
}
void simple_dir_create(struct spdkfs_dir *dir)
{
	dir->dir_persist = malloc(sizeof(struct spdkfs_dir_persist_ctx));
	// Persist data
	dir->dir_persist->d_dirent_count = 0;
	dir->dir_persist->_blob_id = spdk_blob_get_id(dir->blob);
	dir->dir_persist->i_ctime = time(NULL);
	dir->dir_persist->d_size = 0;
	spdk_blob_set_xattr(dir->blob, "dir_persistent", dir->dir_persist,
			    sizeof(struct spdkfs_dir_persist_ctx));

	// In-memory data
	dir->initialized = true;
	dir->dirty = false;

}
