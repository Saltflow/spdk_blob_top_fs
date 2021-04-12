/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "spdk/stdinc.h"
#include "spdk/thread.h"
#include "spdk/bdev.h"
#include "spdk/env.h"
#include "spdk/event.h"
#include "spdk/log.h"
#include "spdk/string.h"
#include "spdk/bdev_module.h"
#include "spdk/blob_bdev.h"
#include "spdk/blob.h"
#include "spdk_fs/fs.h"

static char *g_bdev_name = "Malloc0";

/*
 * We'll use this struct to gather housekeeping hello_context to pass between
 * our events and callbacks.
 */
struct hello_context_t {
	struct spdk_bs_bdev *bdev;
	struct spdk_blob *blob;
	struct spdk_blob_store *bs;
	struct spdk_io_channel *bdev_io_channel;
	char *buff;
	char *bdev_name;
	struct spdk_bdev_io_wait_entry bdev_io_wait;
};

/*
 * Usage function for printing parameters that are specific to this application
 */
static void
hello_bdev_usage(void)
{
	printf(" -b <bdev>                 name of the bdev to use\n");
}

/*
 * This function is called to parse the parameters that are specific to this application
 */
static int hello_bdev_parse_arg(int ch, char *arg)
{
	switch (ch) {
	case 'b':
		g_bdev_name = arg;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

// static void
// base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
// 		   void *event_ctx)
// {
// 	SPDK_WARNLOG("Unsupported bdev event: type %d\n", type);
// }

// void open_blob_complete(void *cb_arg, struct spdk_blob *blb, int bserrno) {
// 	struct hello_context_t* context = cb_arg;
// 	context->blob = blb;
// }

//  void create_complete(void *cb_arg, spdk_blob_id blobid, int bserrno) {
// 	struct hello_context_t* context = cb_arg;
// 	spdk_bs_open_blob(context->bs, blobid, open_blob_complete, context);
//  }

// static void
// bs_init_complete(void *cb_arg, struct spdk_blob_store *bs,
// 		 int bserrno)
// {
// 	struct hello_context_t* context = cb_arg;
// 	context->bs = bs;
// 	spdk_bs_create_blob(bs, create_complete, context);

// }
#include "simple_fs/super.c"

void bridge(void *arg1)
{
	SPDK_WARNLOG("Is there a interception?\n");
	SPDK_WARNLOG("%s \n", spdk_bdev_get_name(spdk_bdev_first()));
	load_simple_spdk_fs();
}

int
main(int argc, char **argv)
{
	struct spdk_app_opts opts = {};
	int rc = 0;
	struct hello_context_t hello_context = {};

	/* Set default values in opts structure. */
	spdk_app_opts_init(&opts);
	opts.name = "hello_bdev";
	opts.reactor_mask = "0x3";


	/*
	 * Parse built-in SPDK command line parameters as well
	 * as our custom one(s).
	 */
	if ((rc = spdk_app_parse_args(argc, argv, &opts, "b:", NULL, hello_bdev_parse_arg,
				      hello_bdev_usage)) != SPDK_APP_PARSE_ARGS_SUCCESS) {
		exit(rc);
	}
	hello_context.bdev_name = g_bdev_name;

	/*
	 * spdk_app_start() will initialize the SPDK framework, call hello_start(),
	 * and then block until spdk_app_stop() is called (or if an initialization
	 * error occurs, spdk_app_start() will return with rc even without calling
	 * hello_start().
	 */

	// pthread_t new_thread;
	// struct bridge_args init_args = {&opts, hello_start, &hello_context};
	// pthread_create(&new_thread, NULL, bridge, &init_args);
	// sem_wait(&g_init_sem);
	// // SPDK_WARNLOG("Initializing finished\n");
	// struct spdk_event* hello_event = spdk_event_allocate(0, hello_start, &hello_context, NULL);
	// spdk_event_call(hello_event);
	// struct spdk_thread* mask_thread =  spdk_thread_create("mask_thread", NULL);
	// spdk_set_thread(mask_thread);

	// spdk_thread_send_msg(g_spdk_app_thread, hello_start, &hello_context);

	// rc = spdk_app_start(&opts, bridge, &hello_context);
	if (rc) {
		SPDK_ERRLOG("ERROR starting application\n");
	}

	/* At this point either spdk_app_stop() was called, or spdk_app_start()
	 * failed because of internal error.
	 */

	/* When the app stops, free up memory that we allocated. */
	spdk_dma_free(hello_context.buff);

	/* Gracefully close out all of the SPDK subsystems. */
	spdk_app_fini();
	return rc;
}
