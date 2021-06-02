#ifndef __spdk_fs_monopoly_op_h__
#define __spdk_fs_monopoly_op_h__

#include "spdkfs/fs.h"
#include "file.h"
#include "dir.h"

int monopoly_open(const char *__file, int __oflag);
int monopoly_close(int __fd);

// NOTE: SPDK Blob only support read/write in io_unit size
ssize_t monopoly_read(int __fd, void *__buf, size_t __nbytes);
ssize_t monopoly_write(int __fd, const void *__buf, size_t __nbytes);
int monopoly_create(const char *__file, int __oflag);

__off_t monopoly_lseek(int __fd, __off_t __offset, int __whence);

int monopoly_stat(const char *__file, struct stat *__buf);

int monopoly_unlink(const char *__filepath);

int monopoly_mkdir(const char *pathname);
int monopoly_chdir(const char *pathname);
#endif
