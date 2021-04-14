#include "interface.h"

static struct fdtable g_spdk_fdtable;
extern struct spdk_filesystem *g_filesystem;

static bool spdk_ptop_blobfile(const char *__file)
{
	if (strlen(__file) < 5) {
		return false;
	}
	if (memcmp(__file, "spdk:", 5)) {
		return false;
	}
	return true;
}

int __spdk__open(const char *__file, int __oflag, ...)
{
	// SPDK_WARNLOG("OPENNING %s\n", __file);
	va_list v_arg_list;
	va_start(v_arg_list, __oflag);
	if (!spdk_ptop_blobfile(__file)) {
		int ret = open(__file, __oflag, v_arg_list);
		va_end(v_arg_list);
		return ret;
	}
	// Create
	if (__oflag | O_CREAT != 0) {
		printf("create !");
		if (g_spdk_fdtable._file_count > SPDK_MAX_FILE_CNT) {
			SPDK_ERRLOG("FD table already full!\n");
			return -1;
		}

	}

}
#define TESTFD 10086

int __spdk__close(int __fd)
{
	if (__fd < TESTFD) {
		return syscall(SYS_close, __fd);
	}
	return -1;
}
ssize_t __spdk_read(int __fd, void *__buf, size_t __nbytes)
{
	if (__fd < TESTFD) {
		return syscall(SYS_read, __fd, __buf, __nbytes);
	}
	return -1;
}
ssize_t __spdk_write(int __fd, const void *__buf, size_t __nbytes)
{
	if (__fd < TESTFD) {
		return syscall(SYS_write, __fd, __buf, __nbytes);
	}
}
__off_t __spdk_lseek(int __fd, __off_t __offset, int __whence)
{
	printf("overiding lseek!!!\n");
	return syscall(SYS_lseek, __fd, __offset, __whence);
}
