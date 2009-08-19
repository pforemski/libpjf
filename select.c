/*
 * select - simpler interface to select(2)
 *
 * This file is part of libasn
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pjf@asn.pl>
 *
 * libasn is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * libasn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "lib.h"

/* used by asn_loop_ */
static thash *fds;
static mmatic *mm;

struct reader {
	char buf[8192];
	int  pos;
	FILE *io;
	void (*cb)(const char *line);
};

thash *asn_rselect(thash *fdlist, uint32_t *timeout_ms, mmatic *mm)
{
	unsigned int fd, nfds = 0;
	struct timeval tv;
	thash *ret;
	fd_set fds;
	void *prv;

	FD_ZERO(&fds);

	thash_reset(fdlist);
	while (THASH_ITER_UINT(fdlist, &fd)) {
		/* XXX: fcntl() is a bit hacky, since we just want to check if fd is valid (so we dont get e.g. a EBADF
		 * immediately after call to select(); if someone knows better method - please fix */
		if (fd >= FD_SETSIZE || fcntl(fd, F_GETFD) < 0) {
			dbg(3, "asn_rselect(): invalid fd %d\n", fd);
			continue;
		}

		FD_SET(fd, &fds);
		nfds = MAX(nfds, fd);
	}

	/* no proper input, no output */
	if (!nfds) return MMTHASH_CREATE_UINT(NULL);

	if (timeout_ms) {
		tv.tv_sec = *timeout_ms / 1000;
		tv.tv_usec = (*timeout_ms % 1000) * 1000;
	}

	dbg(11, "asn_rselect(): calling select() nfds=%d, timeout=%d\n", nfds, *timeout_ms);
	if (select(nfds + 1, &fds, NULL, NULL, (timeout_ms) ? &tv : NULL) < 0) {
		switch (errno) {
			case EBADF:  dbg(0, "asn_rselect(): unexpected EBADF received\n"); break;
			case EINTR:  break; /* signal handled, reload fds as they may have changed */
			case EINVAL: dbg(0, "asn_rselect(): unexpected EINVAL received\n"); break;
			case ENOMEM: die("asn_rselect(): out of memory\n");
			default:     dbg(0, "asn_rselect(): %m\n"); break;
		}

		return NULL;
	}

	ret = MMTHASH_CREATE_UINT(NULL);

	thash_reset(fdlist);
	while ((prv = THASH_ITER_UINT(fdlist, &fd))) {
		if (FD_ISSET(fd, &fds)) {
			dbg(10, "asn_rselect(): fd %d ready\n", fd);
			THASH_SET_UINT(ret, fd, prv); /* magic! */
		}
	}

	if (timeout_ms)
		*timeout_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return ret;
}

void asn_loop_init(mmatic *mymm)
{
	mm = mymm; /* TODO: can be replaced by purely local one? */
	fds = MMTHASH_CREATE_UINT(NULL);
}

void asn_loop_deinit(void)
{
	mmatic_free(mm);
}

void asn_loop(uint32_t timer)
{
	thash *ready;
	uint32_t left;
	struct reader *rd;
	int fd;

	do {
		left = timer;
		ready = asn_rselect(fds, &left, mm);

		if (ready) {
			thash_reset(ready);
			while ((rd = (struct reader *) THASH_ITER_UINT(ready, &fd))) {
read:
				if (fgets(rd->buf + rd->pos, sizeof(rd->buf) - rd->pos, rd->io) == NULL)
					continue;

				/* update pos */
				rd->pos = strlen(rd->buf + rd->pos) + rd->pos; /* XXX: speed-up */

				if (rd->buf[rd->pos - 1] == '\n')
					rd->pos = 0; /* success, we finished reading whole line */
				else
					goto read; /* carry on */

				if (rd->cb)
					rd->cb(rd->buf);
			}
			thash_free(ready);
		}

		if (left > 0)
			usleep(left * 1000);
	} while (true);
}

FILE *asn_loop_connect_tcp(const char *ipaddr, const char *port, void (*cb)(const char *))
{
	int fd;
	FILE *io;
	struct sockaddr_in addr;

	dbg(5, "asn_loop_connect_tcp(%s, %s, %x)\n", ipaddr, port, cb);

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		die_errno("asn_loop_connect_tcp(): socket");

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(atoi(port));

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0)
		die_errno("asn_loop_connect_tcp(): connect");

	dbg(2, "connected to %s:%s\n", ipaddr, port);

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		die_errno("asn_loop_connect_tcp(): fcntl");

	io = fdopen(fd, "w+");
	if (io == NULL)
		die_errno("asn_loop_connect_tcp(): fdopen");

	/* use line-buffered I/O */
	if (setvbuf(io, 0, _IOLBF, 0) != 0)
		die_errno("asn_loop_connect_tcp(): setvbuf");

	/* add to monitored fds */
	THASH_SET_UINT(fds, fd, (void *) mmake(struct reader, "", 0, io, cb));

	return io;
}

FILE *asn_loop_listen_udp(const char *ipaddr, const char *port, void (*cb)(const char *))
{
	int fd;
	FILE *io;
	struct sockaddr_in addr;

	dbg(5, "asn_loop_listen_udp(%s, %s, %x)\n", ipaddr, port, cb);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		die_errno("asn_loop_listen_udp(): socket");

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(atoi(port));

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0)
		die_errno("asn_loop_listen_udp(): connect");

	dbg(2, "bound to %s:%s\n", ipaddr, port);

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		die_errno("asn_loop_listen_udp(): fcntl");

	io = fdopen(fd, "w+");
	if (io == NULL)
		die_errno("asn_loop_listen_udp(): fdopen");

	/* use line-buffered I/O */
	if (setvbuf(io, 0, _IOLBF, 0) != 0)
		die_errno("asn_loop_listen_udp(): setvbuf");

	/* add to monitored fds */
	THASH_SET_UINT(fds, fd, (void *) mmake(struct reader, "", 0, io, cb));

	return io;
}
