#ifndef __spdk_fs_blob_op_h__
#define __spdk_fs_blob_op_h__

#include "spdkfs/fs.h"
#include "file.h"

bool blob_create(struct spdk_blob **blob);

bool blob_open(struct spdk_blob **blob, spdk_blob_id blob_id);

bool blob_close(struct spdk_blob *blob);

bool blob_unlink(spdk_blob_id op_blob_id);

bool generic_blob_io(struct spdk_filesystem *fs, struct spdk_blob *blob, size_t size,
		     loff_t offset, void *buffer, bool read);

bool generic_blob_resize(struct spdk_filesystem *fs, struct spdk_blob *blob, size_t size);

#endif
