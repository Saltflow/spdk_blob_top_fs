
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

struct file_op_cb_args {
	bool *done;
	struct spdk_filesystem *fs;
	enum simple_op_status status;
};

struct simple_fs_cb_args {
	bool *done;
	enum simple_op_status status;
	struct spdk_blob *op_blob;
	spdk_fs_callback cb_fn;
};

void simple_fs_lseek(struct spdkfs_file *, loff_t, int, void *);
void simple_fs_read(struct spdkfs_file *, size_t, loff_t *, void *);
void simple_fs_write(struct spdkfs_file *, size_t, loff_t *, void *);
void simple_fs_open(struct spdk_blob *, struct spdkfs_file *, void *);
void simle_fs_create(struct spdk_blob *, struct spdkfs_file *, void *);
void simple_fs_release(struct spdk_blob *, struct spdkfs_file *, void *);


# endif
