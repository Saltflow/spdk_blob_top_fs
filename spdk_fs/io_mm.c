#include "io_mm.h"
#include "radix_tree.h"
#include <dlfcn.h>

#define SPDK_MM_ENT_OFFSET 996
static bool initialized = false;
static struct radix_tree_root spdk_mem_root = RADIX_TREE_INIT();

struct mm_entry {
	void *ptr;
	bool used;
};

#define VBOX_NVME_IO_UNIT_SIZE 512
#define MAX_IO_PTR_NUM 4096

static struct mm_entry mm_table[MAX_IO_PTR_NUM] = {{0, 0}};
static int mm_table_count = 0;

static inline int find_next_entry()
{
	if (mm_table_count >= MAX_IO_PTR_NUM) {
		return -1;
	}
	for (int i = mm_table_count; i != (mm_table_count  + MAX_IO_PTR_NUM - 1) % MAX_IO_PTR_NUM;
	     i = ((i + 1) % MAX_IO_PTR_NUM)) {
		if (!mm_table[i].used) {
			return i;
		}
	}
	return -1;
}

bool spdkfs_mm_inited()
{
	return initialized;
}

bool spdkfs_mm_init()
{
	if (initialized) {
		return true;
	}
	initialized = true;
	radix_tree_init();
	return true;
}

void *spdkfs_malloc(size_t __size, usr_malloc u_malloc)
{
	if (__size % 4096) {
		void *ptr =  u_malloc(__size);
		assert(ptr);
		return ptr;
	}
	void *spdk_mem = spdk_malloc(__size, VBOX_NVME_IO_UNIT_SIZE, NULL, SPDK_ENV_SOCKET_ID_ANY,
				     SPDK_MALLOC_SHARE);
	if (!spdk_mem) {
		SPDK_ERRLOG("Spdk malloc failed!\n");
		return u_malloc(__size);
	}
	int next_ent =  find_next_entry();
	assert(next_ent != -1);
	mm_table[next_ent].used = true;
	mm_table[next_ent].ptr = spdk_mem;
	radix_tree_insert(&spdk_mem_root, spdk_mem, next_ent + SPDK_MM_ENT_OFFSET);
	return spdk_mem;
}

void *spdkfs_realloc(void *buffer, size_t __size, usr_realloc u_realloc)
{
	if (spdkfs_mm_find(buffer)) {
		return spdk_realloc(buffer, __size, VBOX_NVME_IO_UNIT_SIZE);;
	}
	return u_realloc(buffer, __size);
}

void spdkfs_free(void *ptr, usr_free u_free)
{
	if (spdkfs_mm_find(ptr)) {
		int next_ent = radix_tree_delete(&spdk_mem_root, ptr);
		mm_table[next_ent - SPDK_MM_ENT_OFFSET].used = false;
		spdk_free(ptr);
		return;
	}
	return u_free(ptr);
}

bool spdkfs_mm_find(void *ptr)
{
	if (radix_tree_lookup(&spdk_mem_root, ptr)) {
		return true;
	}
	return false;
}

bool spdkfs_mm_free()
{
	for (int i = 0; i <= MAX_IO_PTR_NUM; ++i) {
		if (!mm_table_count) {
			break;
		}
		if (mm_table[i].used) {
			spdk_free(mm_table[i].ptr);
			mm_table_count--;
		}
	}
	return true;
}
