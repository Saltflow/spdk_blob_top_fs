
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
#define SPDK_MAX_NAME_COUNT 256

struct spdk_super_blob {
	struct spdkfs_dir* root;
	// unnecessary struct spdk_blob* persist_ctx; 
};

struct spdk_filesystem {
	struct spdk_super_blob* super_blob;
	struct spdk_blob_store* bs;
    struct spdk_thread* op_thread;
	
	struct spdk_fs_operations* operations;

};

typedef void(*spdk_fs_callback)(void* cb_arg);


struct spdkfs_file_persist_ctx {
	loff_t			f_pos;
	unsigned int	i_uid;
	unsigned int	i_gid;
	dev_t			i_rdev;
	loff_t			i_size;
	long	i_atime;
	long	i_mtime;
	long	i_ctime;
	size_t f_size;
	unsigned int	i_writecount;
}__attribute__((aligned(4)));

struct spdkfs_file {
	unsigned int 		f_flags;
	struct spdk_blob *_blob;
	struct spdk_filesystem *fs;
	const struct spdk_file_operations	*f_op;
	struct spdkfs_file_persist	*file_persist;

};
struct fdtable {
    // struct pthread_mutex_spinlock lock;
    struct spdkfs_file *open_files[SPDK_MAX_FILE_CNT];
};

struct spdkfs_dir {
	struct blob* blob;
	const struct spdk_file_operations	*d_op;
	unsigned int d_flags;

	TAILQ_HEAD(, spdkfs_dirent) _dirents;

};

struct spdkfs_dirent_persist_ctx {
	char _name[SPDK_MAX_NAME_COUNT];
	spdk_blob_id		id;
	
}__attribute__((aligned(4)));

struct spdkfs_dirent {
	struct spdkfs_dirent_persist_ctx d_ctx;
	struct spdk_dirent *_parent;
	struct spdk_blob *_blob;
};

struct spdk_fs_operations {
	void (*alloc_blob)(struct spdk_filesystem *sb, spdk_fs_callback cb_fn, void* cb_args);
	void (*destroy_blob)(struct spdk_blob *, spdk_fs_callback cb_fn, void* cb_args);
	void (*free_blob)(struct spdk_blob *, spdk_fs_callback cb_fn, void* cb_args);
};

struct spdk_file_operations {
	void (*spdk_lseek) (struct spdkfs_file *, loff_t, int, void*);
	void (*spdk_read) (struct spdkfs_file *, size_t, loff_t *, void*);
	void (*spdk_write) (struct spdkfs_file *, size_t, loff_t *, void*);
	// int (*spdk_mmap) (struct spdkfs_file *, struct vm_area_struct *);
	unsigned long mmap_supported_flags;
	void (*spdk_open) (struct spdk_blob *, struct spdkfs_file *, void*);
	void (*spdk_create) (struct spdk_blob *, struct spdkfs_file *, void*);
	void (*spdk_release) (struct spdk_blob *, struct spdkfs_file *, void*);
};

struct spdk_fs_context {
	struct spdk_filesystem* fs;
	const char* spdk_bdev_name;
	bool* finished;
};

void init_spdk_filesystem(struct spdk_fs_context* fs_ctx);
void cleanup_filesystem(struct spdk_fs_context* fs_ctx);

void spdk_blob_stat(struct spdk_fs_context* fs_ctx);
#endif
