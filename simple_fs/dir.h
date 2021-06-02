
#ifndef __spdk_fs_dir_h__
#define __spdk_fs_dir_h__


#include"spdkfs/fs.h"
#include"file.h"

/**
 * Read the data from the given directory
 *
 * \param dir
\
 */
void simple_dir_read(struct spdkfs_dir *dir);
void simple_dir_write(struct spdkfs_dir *dir);
void simple_dir_close(struct spdkfs_dir *dir);
void simple_dir_create(struct spdkfs_dir *dir);

void bind_dir_ops(struct spdkfs_dir *);


int spdkfs_dir_lookup (const struct spdkfs_dir *dir, const char *name);
#endif