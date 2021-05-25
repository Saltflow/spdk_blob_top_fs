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

int main()
{
	int fd = creat("spdkfs/hello", O_CREAT);
	if (fd == -1) {
		fd = open("spdkfs/hello", O_CREAT);
	}
	assert(fd > 10000);
	char *buffer = malloc(4096);
	memset(buffer, 65, 4096);
	buffer[14] = '\0';
	SPDK_WARNLOG("%s\n", buffer);
	write(fd, buffer, 4096);
	memset(buffer, 66, 4096);
	buffer[14] = '\0';
	SPDK_WARNLOG("%s\n", buffer);
	lseek(fd, 0, SEEK_SET);
	read(fd, buffer, 4096);
	buffer[14] = '\0';
	SPDK_WARNLOG("%s\n", buffer);
	close(fd);
	return 0;
}
