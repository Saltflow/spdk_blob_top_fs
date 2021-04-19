
#ifndef __spdk_fs_file_h__
#define __spdk_fs_file_h__

#include"spdkfs/fs.h"

enum simple_op_status {
	SIMPLE_OP_STATUS_SUCCCESS,

	SIMPLE_OP_STATUS_NO_FREE_SPACE,

	SIMPLE_OP_STATUS_INVALID_PATH,

	SIMPLE_OP_STATUS_INVALID_SPACE,

	SIMPLE_OP_STATUS_UNKNOWN_FAILURE,
};

struct general_op_cb_args {
	struct spdk_filesystem *fs;
	enum simple_op_status status;
	bool *done;
};

struct simple_fs_cb_args {
	struct general_op_cb_args _op;
	struct spdk_blob *op_blob;
	spdk_blob_id blob_id;
	spdk_fs_callback cb_fn;
};

struct simple_fs_dir_ctx {
	int dirent_count;
	struct spdkfs_dirent *dirent_arr;
	struct spdkfs_dir *parent;
	struct general_op_cb_args _op;
};

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
void simple_fs_read(struct spdkfs_file *, size_t, loff_t *, void *);
/**
 * Write the data to the given file
 *
 * \param file file structure to write, as well as to modify metadata
 * \param size size of the writing
 * \param buffer buffer for data
 * \param file_param context for file operations
\
 */

void simple_fs_write(struct spdkfs_file *, size_t, loff_t *, void *);
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
 * Filling the file persistent structure with the give blob
 *
 * \param blob blob to read file metadata
 * \param file file structure to fill
 * \param file_param context for file operations
 *
 */
void simple_fs_release(struct spdk_blob *, struct spdkfs_file *, void *);

/**
 * Read the data from the given directory
 *
 * \param file actually a directory structure, the given directory to read
 * \param size Not applicable
 * \param buffer buffer for data
 * \param dir_param context for dir operations
\
 */
void simple_dir_read(struct spdkfs_file *, size_t, loff_t *, void *);
void simple_dir_write(struct spdkfs_file *, size_t, loff_t *, void *);
void simple_dir_open(struct spdk_blob *, struct spdkfs_file *, void *);
void simple_dir_create(struct spdk_blob *, struct spdkfs_file *, void *);

void bind_dir_ops(struct spdkfs_dir *);

void bind_file_ops(struct spdkfs_file *file);
# endif
