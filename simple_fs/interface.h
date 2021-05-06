#include "file.h"
#include "spdkfs/fs.h"
#include <sys/syscall.h>
#include "spdk/bdev.h"

int __spdk_open(const char *__file, int __oflag, ...);
int __spdk_close(int __fd);
ssize_t __spdk_read(int __fd, void *__buf, size_t __nbytes);
ssize_t __spdk_write(int __fd, const void *__buf, size_t __nbytes);
__off_t __spdk_lseek(int __fd, __off_t __offset, int __whence);
int __spdk_stat(const char *__restrict__ __file, struct stat *__restrict__ __buf);
void *__spdk_malloc(size_t __size);
void __spdk_free(void *__ptr);
void *__spdk_realloc(void *__ptr, size_t __size);

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

void free(void *__ptr) __attribute__((weak, alias("__spdk_free")));

void *realloc(void *__ptr, size_t __size) __attribute__((weak, alias("__spdk_realloc")));
