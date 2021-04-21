#include "io_mm.h"

static bool initialized = false;
static struct spdk_filesystem* curr_filesystem;

bool spdkfs_mm_init(struct spdk_filesystem* fs)
{
    curr_filesystem = fs;
    initialized = true;
}

void* spdkfs_malloc(size_t __size)
{

}

void* spdkfs_realloc(void* buffer, size_t __size)
{

}

void* spdkfs_free(void *ptr)
{

}
