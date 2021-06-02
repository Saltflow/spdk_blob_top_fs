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

static inline size_t get_blob_size(struct spdk_blob* blob, struct spdk_blob_store * bs)
{
	return spdk_blob_get_num_clusters(blob) * spdk_bs_get_cluster_size(bs);
}


ssize_t simple_fs_read(struct spdkfs_file *file, size_t size, void *buffer)
{
	assert(size % 512 == 0);
	if (file->f_pos + size > file->file_persist->f_size) {
		return 0;
	}
	bool ret = generic_blob_io(file->fs, file->_blob, size, file->f_pos, buffer, true);
	if(ret)
	{
		file->f_pos += size;
		spdk_set_thread(file->fs->op_thread);
		spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
					sizeof(struct spdkfs_file_persist_ctx));
		return size;
	}
	return 0;
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
	bool ret = generic_blob_io(file->fs, file->_blob, size, file->f_pos, buffer, false);
	if(ret)
	{
		file->f_pos += size;
		spdk_set_thread(file->fs->op_thread);
		spdk_blob_set_xattr(file->_blob, "file_persistent", file->file_persist,
					sizeof(struct spdkfs_file_persist_ctx));
		return size;
	}
	return 0;
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

void bind_file_ops(struct spdkfs_file *file)
{
	file->f_op = &simplefs_file_ops;
}
