#ifndef __spdk_fs_thread_poller__
#define __spdk_fs_thread_poller__
#include "spdkfs/fs.h"

bool generic_poller(struct spdk_thread* thread, spdk_msg_fn fn, void *ctx, bool *done);

#endif
