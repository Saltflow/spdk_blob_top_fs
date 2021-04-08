
#ifndef __spdk_fs_fs_h__
#define __spdk_fs_fs_h__

#include "spdk/stdinc.h"

#include "spdk/bdev.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/blob_bdev.h"
#include "spdk/blob.h"
#include "spdk/log.h"
#include "spdk/string.h"


#define SPDK_MAX_FILE_CNT 256

struct super_block {
	spdk_blob_id			super_blob;
	struct spdk_blob_store* bs;
	
	struct fs_operations* operations
};

typedef void(*spdk_fs_callback)(void* cb_arg);

struct fs_operations {
	struct spdk_blob *(*alloc_blob)(struct super_block *sb, spdk_fs_callback cb_fn, void* cb_args);
	void (*destroy_blob)(struct blob *, spdk_fs_callback cb_fn, void* cb_args);
	void (*free_blob)(struct blob *, spdk_fs_callback cb_fn, void* cb_args);

};

struct spdkfs_file {
	struct spdk_blob* _blob;
	// struct path		f_path;
	const struct file_operations	*f_op;
	unsigned int 		f_flags;
	loff_t			f_pos;
	// struct fown_struct	f_owner;
	size_t f_size;
};
struct open_file_table {
    // struct pthread_mutex_spinlock lock;
    struct file *open_files[SPDK_MAX_FILE_CNT];
};

struct spdkfs_dir {
	struct blob* blob;
	const struct dir_operations	*d_op;
	unsigned int d_flags;

};

struct spdkfs_dirent {
	char _name[256];
	struct spdk_blob* _blob;
};

#endif