#include "monopoly_ops.h"
#include "blob_op.h"

static struct fdtable g_fdtable;
static struct spdkfs_dir *g_workdir;
extern struct spdk_filesystem *g_filesystem;

static int find_dir(const char *filename, struct spdkfs_dir *_dir)
{
	for (int i = 0; i < _dir->dirent_count; ++i) {
		if (strcmp(filename, _dir->dirents[i]._name) == 0) {
			return i;
		}
	}
	return -1;
}

static int get_fd_from_table()
{
	if(g_fdtable._file_count >= SPDK_MAX_FILE_CNT)
		return -1;
	for(int i = g_fdtable._file_count; i != g_fdtable._file_count-1 ; ++i) {
		if(i == SPDK_MAX_FILE_CNT) {
			i = 0;
		}
		if(!g_fdtable.used[i]) {
			g_fdtable.used[i] = true;
			g_fdtable._file_count++;
			return i;
		}
	}
}

static void return_fd_to_table(int fd)
{
	assert(fd < SPDK_MAX_FILE_CNT);
	g_fdtable._file_count--;
	g_fdtable.used[fd] = false;
}

// Add a dirent to the given dir, this include allocating some memory for persistent
// memory are aligned in io_unit
static bool add_dirent(struct spdk_blob* blob ,const char *filename, struct spdkfs_dir *_dir)
{
    int io_unit = spdk_bs_get_io_unit_size(g_filesystem);
    if(!_dir->dir_mem_cap) {
        _dir->dirents = spdk_malloc(io_unit, io_unit, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
        _dir->dir_mem_cap = io_unit;
    } 
    if (_dir->dir_mem_cap / io_unit <= _dir->dirent_count) {
        _dir->dirents = spdk_realloc(_dir->dirents, _dir->dir_mem_cap + io_unit, io_unit);
		_dir->dir_mem_cap += io_unit;
    }
    struct spdkfs_dirent new_dirent =  _dir->dirents[_dir->dirent_count]; 
	new_dirent._id = spdk_blob_get_id(blob);
	memcpy(new_dirent._name, filename, spdk_min(SPDK_MAX_NAME_COUNT, strlen(filename)));
    _dir->dirent_count++;
}

struct spdkfs_file *monopoly_create(const char *__file, int __oflag)
{
	assert(g_workdir->initialized);
	struct spdkfs_file *new_file = malloc(sizeof(struct spdkfs_file));
	// Find if there exist an identical file
	// If there exists, return the file
	// Otherwise, allocate a blob , then add the blob in the directory
	bind_file_ops(new_file);

	blob_create(&new_file->_blob);
    new_file->f_op->spdk_create(NULL, new_file, NULL);
    add_dirent(new_file->_blob , __file,g_workdir);
    g_fdtable._file_count++;
    g_fdtable.open_files[g_fdtable._file_count] = new_file;
}

int monopoly_open(const char *__file, int __oflag)
{
	int dirent_num = find_dir(__file, g_workdir);
	if (dirent_num == -1) {
		SPDK_ERRLOG("Cannot find file!\n");
		return -1;
	}
	struct spdk_blob *blob;
	blob_open(&blob, g_workdir->dirents[dirent_num]._id);
	struct spdkfs_file *file = malloc(sizeof(struct spdkfs_file));
	bind_file_ops(file);
	file->f_op->spdk_open(blob, file, NULL);
	g_fdtable.open_files[g_fdtable._file_count++] = file;
}
int monopoly_close(int __fd)
{
	g_fdtable.open_files[__fd]->f_op->spdk_close(g_fdtable.open_files, NULL);
	blob_close(g_fdtable.open_files[__fd]->_blob);
	return_fd_to_table(__fd);
}

ssize_t monopoly_read(int __fd, void *__buf, size_t __nbytes)
{
	struct spdkfs_file *file = g_fdtable.open_files[__fd];
	file->f_op->spdk_read(file, __nbytes, __buf, NULL);
}

ssize_t monopoly_write(int __fd, const void *__buf, size_t __nbytes)
{
	struct spdkfs_file *file = g_fdtable.open_files[__fd];
	file->f_op->spdk_write(file, __nbytes, __buf, NULL);
}


__off_t monopoly_lseek(int __fd, __off_t __offset, int __whence)
{

}

