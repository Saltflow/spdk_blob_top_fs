#include "thread_poller.h"

static const int POLLER_MAX_TIME = 1e8;

bool generic_poller(struct spdk_thread *thread, spdk_msg_fn fn, void *ctx, bool *done)
{
	int poller_count = 0;
	spdk_thread_send_msg(thread, fn, ctx);
	do {
		spdk_thread_poll(thread, 0, 0);
		poller_count++;
	} while (!*(done) && poller_count < POLLER_MAX_TIME);
	if (!(*done) && poller_count >= POLLER_MAX_TIME) {
		return false;
	}
	return true;
}
