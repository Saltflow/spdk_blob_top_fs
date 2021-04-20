
#ifndef __spdk_fs_file_h__
#define __spdk_fs_file_h__

#include"spdkfs/fs.h"

/**
 * Modifying the given file's offset
 *
 * \param file file structure to modify
 * \param offset offset to set
 * \param mode offset posistion, now support SEEK_CUR SEEK_SET
 * \param file_param context for file operations
 */

void simple_fs_lseek(struct spdkfs_file *, loff_t, int, void *);

/**
 * Read the data from the given file
 *
 * \param file file structure to read
 * \param size size of the writing
 * \param buffer buffer for data
 * \param file_param context for file operations
\
 */
void simple_fs_read(struct spdkfs_file *, size_t, void *, void *);
/**
 * Write the data to the given file
 *
 * \param file file structure to write, as well as to modify metadata
 * \param size size of the writing
 * \param buffer buffer for data
 * \param file_param context for file operations
\
 */

void simple_fs_write(struct spdkfs_file *, size_t, void *, void *);
/**
 * Filling the file persistent structure with the give blob
 *
 * \param blob blob to read file metadata
 * \param file file structure to fill
 * \param file_param context for file operations
 *
 */
void simple_fs_open(struct spdk_blob *, struct spdkfs_file *, void *);
/**
 * Filling the file persistent structure with the give blob
 *
 * \param blob blob to alloc file meta
 * \param file file structure to fill
 * \param file_param context for file operations
 *
 */
void simple_fs_create(struct spdk_blob *, struct spdkfs_file *, void *);

/**
 * Remove the file's resource
 *
 * \param blob blob to read file metadata
 * \param file file structure to fill
 * \param file_param context for file operations : operation parameters, parent dir blobid
 *
 */
void simple_fs_release(struct spdk_blob *, struct spdkfs_file *, void *);




void simple_fs_close(struct spdkfs_file *file, void *);
/**
 * Read the data from the given directory
 *
 * \param file actually a directory structure, the given directory to read
 * \param size Not applicable
 * \param buffer buffer for data
 * \param dir_param context for dir operations
\
 */
void simple_dir_read(struct spdkfs_dir *dir);
void simple_dir_write(struct spdkfs_dir *dir);
void simple_dir_close(struct spdkfs_dir *dir);
void simple_dir_create(struct spdkfs_dir *dir);

void bind_dir_ops(struct spdkfs_dir *);

void bind_file_ops(struct spdkfs_file *file);
# endif
