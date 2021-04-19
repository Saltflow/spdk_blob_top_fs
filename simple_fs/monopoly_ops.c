#include "monopoly_ops.h"
#include "blob_op.h"

static struct fdtable g_fdtable;
static struct spdkfs_dir* g_workdir;

static int find_dir(const char* filename,struct spdkfs_dir *_dir)
{
    for(int i = 0; i < _dir->dirent_count; ++i)
    {
        if(strcmp(filename, _dir->dirents[i].d_ctx._name) == 0)
        {
            return i;
        }
    }
    return -1;
}

int monopoly_open(const char *__file, int __oflag)
{
    int dirent_num = find_dir(__file, g_workdir);
    if(dirent_num == -1)
    {
        SPDK_ERRLOG("Cannot find file!\n");
        return -1;
    }
    struct spdk_blob* blob;
    blob_open(&blob, g_workdir->dirents[dirent_num].d_ctx.id);
    struct spdkfs_file* file = malloc(sizeof(struct spdkfs_file));
    simple_fs_open(blob, file, NULL);
}
int monopoly_close(int __fd)
{

}

ssize_t monopoly_read(int __fd, void *__buf, size_t __nbytes)
{

}

ssize_t monopoly_write(int __fd, const void *__buf, size_t __nbytes)
{

}


__off_t monopolyk_lseek(int __fd, __off_t __offset, int __whence)
{

}

