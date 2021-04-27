
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

#define UPPER_DIV(a,b) (a - 1) / b + 1

struct spdk_super_blob {
	struct spdkfs_dir *root;
	struct spdk_blob *blob;
};

// FIXME : Potential data racing
struct spdk_io_shared_buffer {
	void *buffer;
	size_t buf_size;
};

struct spdk_filesystem {
	struct spdk_super_blob *super_blob;
	struct spdk_blob_store *bs;
	struct spdk_thread *op_thread;
	struct spdk_io_channel *io_channel;

	struct spdk_fs_operations *operations;

	struct spdk_io_shared_buffer _buffer;


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
	void *xattr;
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
} __attribute__((aligned(4)));

struct spdkfs_dir {
	struct spdk_filesystem *fs;
	struct spdk_blob *blob;
	unsigned int d_flags;
	struct spdkfs_dir *parent;

	const struct spdk_dir_operations	*d_op;

	bool initialized;
	struct spdkfs_dir_persist_ctx *dir_persist;
	struct spdkfs_dirent *dirents;
	bool dirty;
	void *xattr;
};

struct spdkfs_dirent {
	char _name[SPDK_MAX_NAME_COUNT];
	spdk_blob_id		_id;
} __attribute__((aligned(4)));


struct fs_blob_ctx {
	bool *done;
	int fs_errno;
	struct spdk_blob *op_blob;
	spdk_blob_id op_blob_id;
};



struct spdk_fs_operations {
	void (*alloc_blob)(struct spdk_filesystem *sb, struct fs_blob_ctx *);
	void (*free_blob)(struct spdk_blob *, struct fs_blob_ctx *);
};


// All file operations should be perform at upper layer
struct spdk_file_operations {
	void (*spdk_lseek)(struct spdkfs_file *file, loff_t offset, int mode);
	void (*spdk_read)(struct spdkfs_file *file, size_t size, void *buffer);
	void (*spdk_write)(struct spdkfs_file *file, size_t size, void *buffer);

	void (*spdk_open)(struct spdkfs_file *file);
	void (*spdk_close)(struct spdkfs_file *file);
	void (*spdk_create)(struct spdkfs_file *file);
	void (*spdk_release)(struct spdkfs_file *file);
};

struct spdk_dir_operations {
	void (*spdk_mkdir)(struct spdkfs_dir *dir);
	void (*spdk_readdir)(struct spdkfs_dir *dir);
	void (*spdk_writedir)(struct spdkfs_dir *dir);
	void (*spdk_closedir)(struct spdkfs_dir *dir);
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
