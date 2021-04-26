#include "file.h"
#include "spdkfs/fs.h"
#include <sys/syscall.h>
#include "spdk/bdev.h"


int open64(const char *path, int oflag, ...) __attribute__((weak, alias("__spdk_open")));
int open(const char *path, int oflag, ...) __attribute__((weak, alias("__spdk_open")));

int close(int __fd) __attribute__((weak, alias("__spdk_close")));

ssize_t read(int __fd, void *__buf, size_t __nbytes)__attribute__((weak, alias("__spdk_read")));

ssize_t write(int __fd, const void *__buf, size_t __nbytes)__attribute__((weak,
		alias("__spdk_write")));

__off_t lseek(int __fd, __off_t __offset, int __whence) __attribute__((weak,
		alias("__spdk_lseek")));

int stat(const char *__restrict__ __file, struct stat *__restrict__ __buf) __attribute__((weak, alias("__spdk_stat")));

void *malloc (size_t __size) __attribute__((weak, alias("__spdk_malloc")));



void initialize_interface();
