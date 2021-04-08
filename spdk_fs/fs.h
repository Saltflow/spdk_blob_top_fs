
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

struct spdk_filesystem {
	spdk_blob_id			super_blob;
	struct spdk_blob_store* bs;
    struct spdk_thread* op_thread;
	
	struct spdk_fs_operations* operations;

};

typedef void(*spdk_fs_callback)(void* cb_arg);


struct spdkfs_file {
	struct spdk_blob* _blob;
	struct spdk_filesystem* fs;
	// struct path		f_path;
	const struct spdk_file_operations	*f_op;
	unsigned int 		f_flags;
	loff_t			f_pos;
	// struct fown_struct	f_owner;
	size_t f_size;
};
struct open_file_table {
    // struct pthread_mutex_spinlock lock;
    struct spdkfs_file *open_files[SPDK_MAX_FILE_CNT];
};

struct spdkfs_dir {
	struct blob* blob;
	const struct spdk_file_operations	*d_op;
	unsigned int d_flags;

};

struct spdkfs_dirent {
	char _name[256];
	struct spdk_blob* _blob;
};

struct spdk_fs_operations {
	struct spdk_blob *(*alloc_blob)(struct spdk_filesystem *sb, spdk_fs_callback cb_fn, void* cb_args);
	void (*destroy_blob)(struct blob *, spdk_fs_callback cb_fn, void* cb_args);
	void (*free_blob)(struct blob *, spdk_fs_callback cb_fn, void* cb_args);
};

struct spdk_file_operations {
	loff_t (*spdk_lseek) (struct spdkfs_file *, loff_t, int);
	ssize_t (*spdk_read) (struct spdkfs_file *, size_t, loff_t *);
	ssize_t (*spdk_write) (struct spdkfs_file *, size_t, loff_t *);
	int (*spdk_mmap) (struct spdkfs_file *, struct vm_area_struct *);
	unsigned long mmap_supported_flags;
	int (*spdk_open) (struct spdk_blob *, struct spdkfs_file *);
	int (*spdk_release) (struct spdk_blob *, struct spdkfs_file *);
};

void init_spdk_filesystem(bool* done);
void cleanup_filesystem();
bool load_fs_ops(struct spdk_fs_operations *ops);

#endif