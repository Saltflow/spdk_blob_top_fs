#include "io_mm.h"
#include "radix_tree.h"

static bool initialized = false;
static struct spdk_filesystem *curr_filesystem;
static struct radix_tree_root *spdk_mem_root;

bool spdkfs_mm_init(struct spdk_filesystem *fs)
{
	curr_filesystem = fs;
	initialized = true;
}

void *spdkfs_malloc(size_t __size)
{

}

void *spdkfs_realloc(void *buffer, size_t __size)
{

}

void *spdkfs_free(void *ptr)
{

}
