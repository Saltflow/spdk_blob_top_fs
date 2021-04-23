
#ifndef __spdk_fs_file_h__
#define __spdk_fs_file_h__

#include"spdkfs/fs.h"

/**
 * Modifying the given file's offset
 *
 * \param file file structure to modify
 * \param offset offset to set
 * \param mode offset posistion, now support SEEK_CUR SEEK_SET
 */

void simple_fs_lseek(struct spdkfs_file *, loff_t, int);

/**
 * Read the data from the given file
 *
 * \param file file structure to read
 * \param size size of the writing
 * \param buffer buffer for data
\
 */
void simple_fs_read(struct spdkfs_file *, size_t, void *);
/**
 * Write the data to the given file
 *
 * \param file file structure to write, as well as to modify metadata
 * \param size size of the writing
 * \param buffer buffer for data
\
 */

void simple_fs_write(struct spdkfs_file *, size_t, void *);
/**
 * Filling the file persistent structure with the give blob
 * Note that before calling ,file->fs and file->_blob must be properly set
 * \param file file structure to fill
 *
 */
void simple_fs_open(struct spdkfs_file *);
/**
 * Filling the file persistent structure with the give blob
 * Note that before calling ,file->fs and file->_blob must be properly set
 * \param file file structure to fill
 *
 */
void simple_fs_create(struct spdkfs_file *);

/**
 * Remove the file's resource
 *
 * \param file file structure to fill
 *
 */
void simple_fs_release(struct spdkfs_file *);




void simple_fs_close(struct spdkfs_file *file);
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

void bind_file_ops(struct spdkfs_file *file);
# endif
