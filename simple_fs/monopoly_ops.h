#ifndef __spdk_fs_monopoly_op_h__
#define __spdk_fs_monopoly_op_h__

#include "spdkfs/fs.h"
#include "file.h"

int monopoly_open(const char *__file, int __oflag);
int monopoly_close(int __fd);

ssize_t monopoly_read(int __fd, void *__buf, size_t __nbytes);

ssize_t monopoly_write(int __fd, const void *__buf, size_t __nbytes);


__off_t monopoly_lseek(int __fd, __off_t __offset, int __whence);

 int mkdir(const char *pathname, mode_t mode);
#endif
