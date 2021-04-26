#include "monopoly_ops.h"
#include "blob_op.h"

struct fdtable g_fdtable;
struct spdkfs_dir *g_workdir;
extern struct spdk_filesystem *g_filesystem;

static int find_dir(const char *filename, struct spdkfs_dir *_dir)
{
	for (unsigned i = 0; i < _dir->dir_persist->d_dirent_count; ++i) {
		if (strcmp(filename, _dir->dirents[i]._name) == 0) {
			return i;
		}
	}
	return -1;
}

static int get_fd_from_table()
{
	if (g_fdtable._file_count >= SPDK_MAX_FILE_CNT) {
		return -1;
	}
	for (unsigned i = g_fdtable._file_count; i != g_fdtable._file_count - 1 ; ++i) {
		if (i == SPDK_MAX_FILE_CNT) {
			i = 0;
		}
		if (!g_fdtable.used[i]) {
			g_fdtable.used[i] = true;
			g_fdtable._file_count++;
			return i;
		}
	}
	return -1;
}

static void return_fd_to_table(int fd)
{
	assert(fd < SPDK_MAX_FILE_CNT);
	g_fdtable._file_count--;
	g_fdtable.used[fd] = false;
}

// Add a dirent to the given dir, this include allocating some memory for persistent
// memory are aligned in io_unit
static bool add_dirent(struct spdk_blob *blob, const char *filename, struct spdkfs_dir *_dir)
{
	int io_unit = spdk_bs_get_io_unit_size(g_filesystem->bs);
	_dir->dirty = true;
	if (!_dir->dir_persist->d_size) {
		_dir->dirents = spdk_malloc(io_unit, io_unit, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
		_dir->dir_persist->d_size = io_unit;
	}
	if (_dir->dir_persist->d_size / io_unit <= _dir->dir_persist->d_dirent_count * sizeof(
		    struct spdkfs_dirent)) {
		_dir->dirents = spdk_realloc(_dir->dirents, _dir->dir_persist->d_size + io_unit, io_unit);
		_dir->dir_persist->d_size += io_unit;
	}
	struct spdkfs_dirent *new_dirent =  _dir->dirents + _dir->dir_persist->d_dirent_count;
	new_dirent->_id = spdk_blob_get_id(blob);
	memcpy(new_dirent->_name, filename, spdk_min(SPDK_MAX_NAME_COUNT, strlen(filename)));
	_dir->dir_persist->d_dirent_count++;
}

// Find if there exist an identical file
// If there exists, return the file
// Otherwise, allocate a blob , then add the blob in the directory
int monopoly_create(const char *__file, int __oflag)
{
	assert(g_workdir->initialized);
	struct spdkfs_file *new_file = malloc(sizeof(struct spdkfs_file));

	if (find_dir(__file, g_workdir) != -1) {
		SPDK_ERRLOG("File already exist!\n");
		return -1;
	}
	bind_file_ops(new_file);
	new_file->fs = g_filesystem;
	blob_create(&new_file->_blob);
	new_file->f_op->spdk_create(new_file);
	add_dirent(new_file->_blob, __file, g_workdir);
	int new_fd = get_fd_from_table();
	if (new_fd == -1) {
		SPDK_ERRLOG("Fd table already full!\n");
		return -1;
	}
	g_fdtable.open_files[new_fd] = new_file;
	return new_fd;
}

int monopoly_open(const char *__file, int __oflag)
{
	int dirent_num = find_dir(__file, g_workdir);
	if (dirent_num == -1) {
		SPDK_ERRLOG("Cannot find file!\n");
		return -1;
	}
	struct spdkfs_file *file = malloc(sizeof(struct spdkfs_file));
	blob_open(&file->_blob, g_workdir->dirents[dirent_num]._id);
	bind_file_ops(file);
	file->fs = g_filesystem;
	file->f_op->spdk_open(file);
	int new_fd = get_fd_from_table();
	if (new_fd == -1) {
		SPDK_ERRLOG("Fd table already full!\n");
		return -1;
	}
	g_fdtable.open_files[new_fd] = file;
	return new_fd;
}
int monopoly_close(int __fd)
{
	g_fdtable.open_files[__fd]->f_op->spdk_close(g_fdtable.open_files[__fd]);
	return_fd_to_table(__fd);
	return 0;
}

// Note: blob have restricted that any size and offset be the multiple of io_unit_size
ssize_t monopoly_read(int __fd, void *__buf, size_t __nbytes)
{
	assert(__fd != -1);
	struct spdkfs_file *file = g_fdtable.open_files[__fd];
	int io_unit = spdk_bs_get_io_unit_size(file->fs->bs);
	if (__nbytes % io_unit) {
		__nbytes = UPPER_DIV(__nbytes, io_unit);
	}
	file->f_op->spdk_read(file, __nbytes, __buf);
	return __nbytes;
}

// Note: blob have restricted that any size and offset be the multiple of io_unit_size
ssize_t monopoly_write(int __fd, const void *__buf, size_t __nbytes)
{
	assert(__fd != -1);
	struct spdkfs_file *file = g_fdtable.open_files[__fd];
	int io_unit = spdk_bs_get_io_unit_size(file->fs->bs);
	if (__nbytes % io_unit) {
		__nbytes = UPPER_DIV(__nbytes, io_unit);
	}
	file->f_op->spdk_write(file, __nbytes, __buf);
	return __nbytes;
}


__off_t monopoly_lseek(int __fd, __off_t __offset, int __whence)
{
	assert(__fd != -1);
	struct spdkfs_file *file = g_fdtable.open_files[__fd];
	int io_unit = spdk_bs_get_io_unit_size(file->fs->bs);
	__off_t actual_seek = __offset / io_unit * io_unit;
	if (__whence == SEEK_SET) {
		file->f_pos = actual_seek;
	}
	if (__whence == SEEK_CUR) {
		file->f_pos += actual_seek;
	}
	return file->f_pos;
}

int monopoly_stat(const char * __file, struct stat * __buf)
{
	int target_file_id = find_dir(__file, g_workdir);
	if(target_file_id == -1) {
		return -1;
	}
	struct spdk_blob* stat_blob;
	if(!blob_open(&stat_blob, target_file_id))
		return -1;

	struct spdkfs_file_persist_ctx *file_meta;
	size_t len;
	spdk_blob_get_xattr_value(stat_blob, "file_persistent", &file_meta, &len);
	assert(len == sizeof(struct spdkfs_file_persist_ctx));
	__buf->st_ino = spdk_blob_get_id(stat_blob);
	__buf->st_size = file_meta->f_size;
	__buf->st_mtime = file_meta->i_mtime;
	__buf->st_atime = file_meta->i_atime;
	__buf->st_blksize = spdk_bs_get_io_unit_size(g_workdir->fs->bs);
	__buf->st_uid = file_meta->i_uid;
	return 0;
}
