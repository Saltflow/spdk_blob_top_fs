#include "interface.h"
#include "blob_op.h"
#include <dlfcn.h>
#include "monopoly_ops.h"
#define __GNU_SOURCE

#define TESTFD 10086

extern struct fdtable g_fdtable;
extern struct spdk_filesystem *g_filesystem;

static volatile struct spdk_blob *test_blob = NULL;
static volatile char *general_buffer = NULL;

static bool spdk_ptop_blobfile(const char *__file);

static int (*r_open)(const char *__file, int __oflag, ...) = NULL;
static int (*r_close)(int __fd) = NULL;
static ssize_t (*r_read)(int __fd, void *__buf, size_t __nbytes) = NULL;
static ssize_t (*r_write)(int __fd, const void *__buf, size_t __nbytes) = NULL;
static __off_t (*r_lseek)(int __fd, __off_t __offset, int __whence) = NULL;

void initialize_interface()
{
	// r_malloc = dlsym(RTLD_NEXT, "malloc");
	r_open = dlsym(RTLD_NEXT, "open64");
	r_close = dlsym(RTLD_NEXT, "close");
	r_read = dlsym(RTLD_NEXT, "read");
	r_write = dlsym(RTLD_NEXT, "write");
	r_lseek = dlsym(RTLD_NEXT, "lseek");
}


static bool spdk_ptop_blobfile(const char *__file)
{
	if (strlen(__file) < 5) {
		return false;
	}
	if (memcmp(__file, "spdkdir/", 8)) {
		return false;
	}
	return true;
}

static loff_t blob_offset;

char *files[1000];

int __spdk__open(const char *__file, int __oflag, ...)
{
	SPDK_WARNLOG("OPENNING %s\n", __file);
	va_list v_arg_list;
	va_start(v_arg_list, __oflag);
	if (!spdk_ptop_blobfile(__file)) {
		int ret = r_open(__file, __oflag, v_arg_list);
		printf("%d %s\n", ret, __file);
		va_end(v_arg_list);
		return ret;
	}
	// Create
	if (__oflag | O_CREAT != 0) {
		printf("create !");
		if (g_fdtable._file_count > SPDK_MAX_FILE_CNT) {
			SPDK_ERRLOG("FD table already full!\n");
			return -1;
		}
		general_buffer =  spdk_malloc(2098176, 4096, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
		int ret = monopoly_create(__file, __oflag);
		if(ret == -1)
		{
			return TESTFD + monopoly_open(__file, __oflag);
		}
		return TESTFD + ret;
	}
	spdk_blob_id open_id;
	return TESTFD + monopoly_open(__file, __oflag);
}

int __spdk__close(int __fd)
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
	int ret = monopoly_read(__fd  - TESTFD, general_buffer, __nbytes);
	memcpy(__buf, general_buffer, __nbytes);
	return ret;
}
ssize_t __spdk_write(int __fd, const void *__buf, size_t __nbytes)
{
	if (__fd < TESTFD) {
		return syscall(SYS_write, __fd, __buf, __nbytes);
	}
	size_t io_unit =  spdk_bs_get_io_unit_size(g_filesystem->bs);
	size_t io_size = ((__nbytes - 1) / io_unit + 1) * io_unit;

	memcpy(general_buffer, __buf, io_size);
	return monopoly_write(__fd  - TESTFD, general_buffer, __nbytes);
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
			stat(__file, __buf);
		}
		monopoly_stat(__file, __buf);
}

// void *__spdk__malloc(size_t __size)
// {
// 	if(__size % 4096)
// 	{
//     	return r_malloc(__size);
// 	}
// 	if(g_filesystem)
// 		spdk_malloc(__size, 4096, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_SHARE);
// 	else
// 		return r_malloc(__size);
// }
