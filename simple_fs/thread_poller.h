#ifndef __spdk_fs_thread_poller__
#define __spdk_fs_thread_poller__
#include "spdk_fs/fs.h"

bool generic_poller(spdk_msg_fn fn, void *ctx);

#endif