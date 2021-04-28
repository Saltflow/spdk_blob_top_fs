#include "spdk/bdev.h"
#include "spdk/accel_engine.h"
#include "spdk/env.h"
#include "spdk/thread.h"
#include "spdk/log.h"
#include "spdk/string.h"
#include "spdk/queue.h"
#include "spdk/util.h"

#include "spdk_internal/thread.h"
#include "spdk_internal/event.h"

#include "spdkfs/fs.h"

#include"../../simple_fs/monopoly_ops.h"
int main()
{
	int fd = monopoly_create("spdkfs/hello", O_CREAT);
	if (fd == -1) {
		fd = monopoly_open("spdkfs/hello", O_CREAT);
	}
	char *buffer = malloc(4096);
	memset(buffer, 65, 4096);
	buffer[14] = '\0';
	SPDK_WARNLOG("%s\n", buffer);
	monopoly_write(fd, buffer, 4096);
	memset(buffer, 66, 4096);
	buffer[14] = '\0';
	SPDK_WARNLOG("%s\n", buffer);
	monopoly_lseek(fd, 0, SEEK_SET);
	monopoly_read(fd, buffer, 4096);
	buffer[14] = '\0';
	SPDK_WARNLOG("%s\n", buffer);
	monopoly_close(fd);
	return 0;
}
