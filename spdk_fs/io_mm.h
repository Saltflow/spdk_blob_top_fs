#ifndef __spdk_fs_io_mm_h__
#define __spdk_fs_io_mm_h__

#include "fs.h"

typedef void *(*usr_malloc)(size_t);
typedef void (*usr_free)(void *ptr);
typedef void *(*usr_realloc)(void *__ptr, size_t __size);

bool spdkfs_mm_inited();

bool  spdkfs_mm_init();

void *spdkfs_malloc(size_t __size, usr_malloc u_malloc);

void *spdkfs_realloc(void *buffer, size_t __size, usr_realloc u_malloc);

void spdkfs_free(void *ptr, usr_free u_free);

bool spdkfs_mm_find(void *ptr);

bool spdkfs_mm_free();

#endif
