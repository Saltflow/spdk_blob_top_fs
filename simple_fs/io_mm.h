#ifndef __spdk_fs_io_mm_h__
#define __spdk_fs_io_mm_h__

#include "spdkfs/fs.h"

bool spdkfs_mm_init(struct spdk_filesystem* fs);

void* spdkfs_malloc(size_t __size);

void* spdkfs_realloc(void* buffer, size_t __size);

void* spdkfs_free(void *ptr);

#endif
