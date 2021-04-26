#include "io_mm.h"
#include "radix_tree.h"
#include <dlfcn.h>

static bool initialized = false;
static struct spdk_filesystem *curr_filesystem;
static struct radix_tree_root spdk_mem_root = RADIX_TREE_INIT();

static void *(*r_malloc)(size_t) = NULL;
static void (*r_free)(void *ptr) = NULL;


bool spdkfs_mm_inited()
{
	return initialized;
}

bool spdkfs_mm_init(struct spdk_filesystem *fs)
{
	r_malloc = dlsym(RTLD_NEXT, "malloc");
	r_free = dlsym(RTLD_NEXT, "free");
	curr_filesystem = fs;
	initialized = true;
	radix_tree_init();
	return true;
}

void *spdkfs_malloc(size_t __size)
{
	assert(r_malloc);
	if (__size % 4096) {
		return r_malloc(__size);
	}
	void *spdk_mem = spdk_malloc(__size, 512, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
	radix_tree_insert(&spdk_mem_root, spdk_mem, NULL);
}

void *spdkfs_realloc(void *buffer, size_t __size)
{
	assert(r_malloc);
}

void spdkfs_free(void *ptr)
{
	if (radix_tree_lookup(&spdk_mem_root, ptr)) {
		radix_tree_delete(&spdk_mem_root, ptr);
		spdk_free(ptr);
		return;
	}
	return r_free(ptr);
}

bool spdkfs_mm_free()
{
	char *left_mem;
	int left_item_count = 0;
	while (left_item_count =  radix_tree_gang_lookup(&spdk_mem_root, &left_mem, 0, 128)) {
		for (int i = 0; i < left_item_count; ++i) {
			radix_tree_delete(&spdk_mem_root, left_mem[i]);
		}
	}

}
