#include "interface.h"
#include "blob_op.h"
#include <dlfcn.h>
#include "monopoly_ops.h"
#include "spdkfs/io_mm.h"
#define __GNU_SOURCE

#define TESTFD 10086

extern struct fdtable g_fdtable;
extern struct spdk_filesystem *g_filesystem;

static bool spdk_ptop_blobfile(const char *__file);

static int (*r_open)(const char *__file, int __oflag, ...) = NULL;
static int (*r_stat)(const char *__restrict__ __file, struct stat *__restrict__ __buf) = NULL;

void initialize_interface()
{
	r_open = dlsym(RTLD_NEXT, "open64");
	r_stat = dlsym(RTLD_NEXT, "stat");
}


static bool spdk_ptop_blobfile(const char *__file)
{
	if (strlen(__file) < 5) {
		return false;
	}
	if (memcmp(__file, "spdkfs/", 7)) {
		return false;
	}
	return true;
}

int __spdk_open(const char *__file, int __oflag, ...)
{
	SPDK_WARNLOG("OPENNING %s\n", __file);
	va_list v_arg_list;
	va_start(v_arg_list, __oflag);
	if (!spdk_ptop_blobfile(__file)) {
		int ret = r_open(__file, __oflag, v_arg_list);
		va_end(v_arg_list);
		return ret;
	}
	// Create
	if ((__oflag | O_CREAT) != 0) {
		if (g_fdtable._file_count > SPDK_MAX_FILE_CNT) {
			SPDK_ERRLOG("FD table already full!\n");
			return -1;
		}
		int ret = monopoly_create(__file, __oflag);
		if(ret == -1)
		{
			return TESTFD + monopoly_open(__file, __oflag);
		}
		return TESTFD + ret;
	}
	return TESTFD + monopoly_open(__file, __oflag);
}

int __spdk_close(int __fd)
{
	if (__fd < TESTFD) {
		return syscall(SYS_close, __fd);
	}
	return monopoly_close(__fd - TESTFD);
}

ssize_t __spdk_read(int __fd, void *__buf, size_t __nbytes)
{
	if (__fd < TESTFD) {
		return syscall(SYS_read, __fd, __buf, __nbytes);
	}
	int ret = monopoly_read(__fd  - TESTFD, __buf, __nbytes);
	return ret;
}
ssize_t __spdk_write(int __fd, const void *__buf, size_t __nbytes)
{
	if (__fd < TESTFD) {
		return syscall(SYS_write, __fd, __buf, __nbytes);
	}
	return monopoly_write(__fd  - TESTFD, __buf, __nbytes);
}
__off_t __spdk_lseek(int __fd, __off_t __offset, int __whence)
{
	if (__fd < TESTFD) {
		return syscall(SYS_lseek, __fd, __offset, __whence);
	}
	return monopoly_lseek(__fd  - TESTFD, __offset, __whence);
}

int __spdk_stat(const char *__restrict__ __file, struct stat *__restrict__ __buf)
{
	if (!spdk_ptop_blobfile(__file)) {
		return syscall(SYS_stat, __file, __buf);
	}
	return monopoly_stat(__file, __buf);
}

static void *(*r_malloc)(size_t) = NULL;
static void (*r_free)(void *__ptr) = NULL;
static void *(*r_realloc)(void *__ptr, size_t __size) = NULL;

void *__spdk_malloc(size_t __size)
{
	if(r_malloc == NULL) {
		r_malloc = dlsym(RTLD_NEXT, "malloc");
	}
	if(!spdkfs_mm_inited()) {
		return r_malloc(__size);
	} else {
		return spdkfs_malloc(__size);
	}
}

void __spdk_free(void *__ptr)
{
	if(r_free == NULL) {
		r_free = dlsym(RTLD_NEXT, "free");
	}
	if(!spdkfs_mm_inited()) {
		return r_free(__ptr);
	} else {
		return spdkfs_free(__ptr);
	}
}

void *__spdk_realloc(void *__ptr, size_t __size)
{
	if(r_realloc == NULL) {
		r_realloc = dlsym(RTLD_NEXT, "realloc");
	}
	if(!spdkfs_mm_inited()) {
		return r_realloc(__ptr, __size);
	} else {
		return spdkfs_realloc(__ptr, __size);
	}
}


int __spdk_unlink(const char *pathname) {
	if (!spdk_ptop_blobfile(pathname)) {
		return syscall(SYS_unlink, pathname);
	}
	return monopoly_unlink(pathname);
}
