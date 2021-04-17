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
#include "config-host.h"
#include "fio.h"
#include "optgroup.h"

#include"simple_fs/interface.h"

int spdk_open_file(struct thread_data *td, struct fio_file *f)
{
	f->fd = __spdk__open(f->file_name, O_CREAT, 0600);
	printf("%d\n", f->fd);
	return 0;
}

int spdk_get_file_size(struct thread_data *td, struct fio_file *f)
{
	f->real_file_size = 2000000;
	fio_file_set_size_known(f);
	return 0;
}

int spdk_close_file(struct thread_data fio_unused *td, struct fio_file *f)
{
	__spdk__close(f->fd);
	
	f->fd = -1;
	return 0;
}

static int fio_spdk_syncio_prep(struct thread_data *td, struct io_u *io_u)
{
	__spdk_lseek(io_u->file->fd, 0, SEEK_SET);
	return 0;
}

#define LAST_POS(f)	((f)->engine_pos)

static int fio_io_end(struct thread_data *td, struct io_u *io_u, int ret)
{
	if (io_u->file && ret >= 0 && ddir_rw(io_u->ddir))
		LAST_POS(io_u->file) = io_u->offset + ret;

	if (ret != (int) io_u->xfer_buflen) {
		if (ret >= 0) {
			io_u->resid = io_u->xfer_buflen - ret;
			io_u->error = 0;
			return FIO_Q_COMPLETED;
		} else
			io_u->error = errno;
	}

	if (io_u->error) {
		io_u_log_error(td, io_u);
		td_verror(td, io_u->error, "xfer");
	}

	return FIO_Q_COMPLETED;
}

static enum fio_q_status fio_spdk_syncio_queue(struct thread_data *td,
					  struct io_u *io_u)
{
	struct fio_file *f = io_u->file;
	int ret;

	fio_ro_check(td, io_u);

	if (io_u->ddir == DDIR_READ)
		ret = __spdk_read(f->fd, io_u->xfer_buf, io_u->xfer_buflen);
	else if (io_u->ddir == DDIR_WRITE)
		ret = __spdk_write(f->fd, io_u->xfer_buf, io_u->xfer_buflen);
	else if (io_u->ddir == DDIR_TRIM) {
		do_io_u_trim(td, io_u);
		return FIO_Q_COMPLETED;
	} else
		ret = 0;

	return fio_io_end(td, io_u, ret);
}

static int
spdkfs_fio_invalidate(struct thread_data *td, struct fio_file *f)
{
	/* TODO: This should probably send a flush to the device, but for now just return successful. */
	return 0;
}

static struct ioengine_ops ioengine_spdk_rw = {
	.name		= "spdk_sync",
	.version	= FIO_IOOPS_VERSION,
	.flags			= FIO_RAWIO | FIO_NOEXTEND | FIO_NODISKUTIL | FIO_MEMALIGN,
	.prep		= fio_spdk_syncio_prep,
	.queue		= fio_spdk_syncio_queue,
	.open_file	= spdk_open_file,
	.close_file	= spdk_close_file,
	.get_file_size	= spdk_get_file_size,
	.invalidate = spdkfs_fio_invalidate,
	.flags		= FIO_SYNCIO,
};


static void fio_init spdk_fio_register(void)
{
	register_ioengine(&ioengine_spdk_rw);
}
static void fio_exit spdk_fio_unregister(void)
{
	unregister_ioengine(&ioengine_spdk_rw);
}
