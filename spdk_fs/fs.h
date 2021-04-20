
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
#define SPDK_MAX_NAME_COUNT 50

struct spdk_super_blob {
	struct spdkfs_dir *root;
	struct spdk_blob *blob;
};

struct spdk_filesystem {
	struct spdk_super_blob *super_blob;
	struct spdk_blob_store *bs;
	struct spdk_thread *op_thread;
	struct spdk_io_channel *io_channel;

	struct spdk_fs_operations *operations;

	TAILQ_ENTRY(spdk_blob *)	open_blob;
};

typedef void(*spdk_fs_callback)(void *cb_arg);


struct spdkfs_file_persist_ctx {
	unsigned int	i_uid;
	unsigned int	i_gid;
	long	i_atime;
	long	i_mtime;
	long	i_ctime;
	size_t f_size;
	unsigned int	i_writecount;
	uint64_t _blob_id;
	bool dirty;
} __attribute__((aligned(4)));

struct spdkfs_file {
	unsigned int 		f_flags;
	struct spdk_blob *_blob;
	struct spdk_filesystem *fs;
	const struct spdk_file_operations	*f_op;
	struct spdkfs_file_persist_ctx	*file_persist;

	loff_t			f_pos;

};
struct fdtable {
	unsigned int _file_count;
	bool used[SPDK_MAX_FILE_CNT];
	struct spdkfs_file *open_files[SPDK_MAX_FILE_CNT];
};


struct spdkfs_dir_persist_ctx {
	unsigned int	mode;
	long	i_atime;
	long	i_mtime;
	long	i_ctime;
	size_t d_dirent_count;
	size_t d_size;
	uint64_t _blob_id;
	bool dirty;
} __attribute__((aligned(4)));

struct spdkfs_dir {
	struct spdk_filesystem* fs;
	struct blob *blob;
	unsigned int d_flags;
	struct spdkfs_dir* parent;

	const struct spdk_dir_operations	*d_op;

	bool initialized;
	struct spdkfs_dir_persist_ctx *dir_persist;
	struct spdkfs_dirent *dirents;
	int dir_mem_cap;
	int dirent_count;

};

struct spdkfs_dirent {
	char _name[SPDK_MAX_NAME_COUNT];
	spdk_blob_id		_id;
} __attribute__((aligned(4)));


struct spdk_fs_operations {
	void (*alloc_blob)(struct spdk_filesystem *sb, spdk_fs_callback cb_fn, void *cb_args);
	void (*destroy_blob)(struct spdk_blob *, spdk_fs_callback cb_fn, void *cb_args);
	void (*free_blob)(struct spdk_blob *, spdk_fs_callback cb_fn, void *cb_args);
};

// All file operations should be perform at upper layer
struct spdk_file_operations {
	void (*spdk_lseek)(struct spdkfs_file *file, loff_t offset, int mode, void *);
	void (*spdk_read)(struct spdkfs_file *file, size_t size, void *buffer, void *fs_ctx);
	void (*spdk_write)(struct spdkfs_file *file, size_t size, void *buffer, void *fs_ctx);
	// int (*spdk_mmap) (struct spdkfs_file *, struct vm_area_struct *);
	unsigned long mmap_supported_flags;
	void (*spdk_open)(struct spdk_blob *blob, struct spdkfs_file *file, void *);
	void (*spdk_close)(struct spdkfs_file *file, void *);
	void (*spdk_create)(struct spdk_blob *blob, struct spdkfs_file *file, void *);
	void (*spdk_release)(struct spdk_blob *blob, struct spdkfs_file *file, void *);
};

struct spdk_dir_operations {
	void (*spdk_mkdir)		(struct spdkfs_dir* dir);
	void (*spdk_readdir)	(struct spdkfs_dir* dir);
	void (*spdk_writedir)	(struct spdkfs_dir* dir);
	void (*spdk_closedir)	(struct spdkfs_dir* dir);
};

struct spdk_fs_init_ctx {
	struct spdk_filesystem *fs;
	const char *spdk_bdev_name;
	bool *finished;
};


struct spdk_fs_generic_ctx {
	struct spdk_filesystem *fs;
	void *args;
	bool *done;
};

void init_spdk_filesystem(struct spdk_fs_init_ctx *fs_ctx);
void cleanup_filesystem(struct spdk_fs_init_ctx *fs_ctx);

void spdk_blob_stat(struct spdk_fs_init_ctx *fs_ctx);
#endif
