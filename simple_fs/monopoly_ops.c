#include "monopoly_ops.h"
#include "blob_op.h"

static struct fdtable g_fdtable;
static struct spdkfs_dir *g_workdir;

static int find_dir(const char *filename, struct spdkfs_dir *_dir)
{
	for (int i = 0; i < _dir->dirent_count; ++i) {
		if (strcmp(filename, _dir->dirents[i].d_ctx._name) == 0) {
			return i;
		}
	}
	return -1;
}

struct spdkfs_file *monopoly_create(const char *__file, int __oflag)
{
	assert(g_workdir->initialized);
	struct spdkfs_file *new_file = malloc(sizeof(struct spdkfs_file));
	// Find if there exist an identical file
	// If there exists, return the file
	// Otherwise, allocate a blob , then sync the blob in the directory
	bind_file_ops(new_file);
	if (g_workdir->dirent_count == 0) {
		blob_create(&new_file->_blob);
		g_workdir->dirent_count++;
		struct spdkfs_dirent *new_dir = spdk_malloc(sizeof(struct spdkfs_dirent), 0, NULL,
						SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
		g_workdir->d_op->spdk_write(g_workdir, sizeof(struct spdkfs_dirent), new_dir, NULL);
	}

}

struct spdkfs_file *monopoly_open(const char *__file, int __oflag)
{
	int dirent_num = find_dir(__file, g_workdir);
	if (dirent_num == -1) {
		SPDK_ERRLOG("Cannot find file!\n");
		return -1;
	}
	struct spdk_blob *blob;
	blob_open(&blob, g_workdir->dirents[dirent_num].d_ctx.id);
	struct spdkfs_file *file = malloc(sizeof(struct spdkfs_file));
	bind_file_ops(file);
	file->f_op->spdk_open(blob, file, NULL);
}
int monopoly_close(int __fd)
{
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

