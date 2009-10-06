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
static mmatic *mm;
static thash *fds;           /** fds monitored in main loop via asn_rselect() */

struct reader {
	bool isnet;
	char buf[8192];
	int  pos; /* XXX: pos = recv() - dont unsign it */
	FILE *io;
	void (*cb)(const char *line);
};

struct sender {
	const char *interface;
	int fd;
	struct sockaddr_in addr;
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
				if (rd->isnet) {
					rd->pos = recv(fd, rd->buf, sizeof(rd->buf), 0);

					if (rd->pos > 0)
						rd->buf[rd->pos] = '\0';
					else
						continue;
				}
				else {
					int ch;
					errno = 0;
					ch = fgetc(rd->io);
					if (ch == EOF)
						continue;
					rd->buf[rd->pos++] = ch;

					if (ch == '\n') {
						rd->buf[rd->pos] = '\0';
						rd->pos = 0; /* success, we finished reading whole line */
					} else
						goto read; /* carry on */
				}

				if (rd->cb)
					rd->cb(rd->buf);

				goto read;
			}
			thash_free(ready);
		}

		/* TODO: add support for periodical calls of fn-s */
		/* TODO: add support for timeout-ed calls of fn-s */

		if (left > 0)
			usleep(left * 1000);
	} while (true);
}

void asn_loop_add_fd(int fd, loop_cb cb)
{
	FILE *io;
	long int fl;

	fl = fcntl(fd, F_GETFL);
	if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0)
		die_errno("asn_loop_add_fd(): fcntl");

	io = fdopen(fd, "r");
	if (io == NULL)
		die_errno("asn_loop_add_fd(): fdopen");

	/* use line-buffered I/O */
	if (setvbuf(io, 0, _IOLBF, 0) != 0)
		die_errno("asn_loop_add_fd(): setvbuf");

	/* add to monitored fds */
	THASH_SET_UINT(fds, fd, (void *) mmake(struct reader, false, "", 0, io, cb));
}

FILE *asn_loop_connect_tcp(const char *ipaddr, const char *port, loop_cb cb)
{
	int fd;
	FILE *io;
	struct sockaddr_in addr;

	dbg(5, "asn_loop_connect_tcp(%s, %s, 0x%x)\n", ipaddr, port, cb);

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
	THASH_SET_UINT(fds, fd, (void *) mmake(struct reader, true, "", 0, io, cb));

	return io;
}

int asn_loop_listen_udp(const char *iface, const char *ipaddr, const char *port, loop_cb cb)
{
	int fd;
	struct sockaddr_in addr;
	int one = 1;

	dbg(5, "asn_loop_listen_udp(%s, %s, %s, 0x%x)\n", iface, ipaddr, port, cb);

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0)
		die_errno("asn_loop_listen_udp(): socket");

#if 0
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0)
		die_errno("asn_loop_listen_udp(): SO_REUSEADDR");
#endif

	if (iface && *iface && setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) < 0)
		die_errno("asn_loop_listen_udp(): SO_BINDTODEVICE");

	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		die_errno("asn_loop_listen_udp(): fcntl");

	memset((char *) &addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ipaddr);
	addr.sin_port = htons(atoi(port));

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) != 0)
		die_errno("asn_loop_listen_udp(): connect");

	dbg(2, "bound to %s:%s\n", ipaddr, port);

	/* add to monitored fds */
	THASH_SET_UINT(fds, fd, (void *) mmake(struct reader, true, "", 0, NULL, cb));

	return fd;
}

void *asn_loop_udp_sender(const char *iface, const char *ipaddr, const char *port)
{
	int one = 1;
	struct sender *s = mmake(struct sender, iface, 0, { 0 });

	dbg(5, "asn_loop_udp_sender(%s, %s, %s)\n", iface, ipaddr, port);

	s->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s->fd < 0)
		die_errno("asn_udp_send_create(): socket");

	if (setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) < 0)
		die_errno("asn_udp_send_create(): SO_REUSEADDR");

	if (iface && *iface && setsockopt(s->fd, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) < 0)
		die_errno("asn_udp_send_create(): SO_BINDTODEVICE");

	if (setsockopt(s->fd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(int)) < 0)
		die_errno("asn_udp_send_create(): SO_BROADCAST");

	if (fcntl(s->fd, F_SETFL, O_NONBLOCK) < 0)
		die_errno("asn_udp_send_create(): fcntl");

	memset((char *) &(s->addr), 0, sizeof(struct sockaddr_in));
	s->addr.sin_family = AF_INET;
	s->addr.sin_addr.s_addr = inet_addr(ipaddr);
	s->addr.sin_port = htons(atoi(port));

	return s;
}

void asn_loop_send_udp(void *sender, const char *line)
{
	struct sender *s = (struct sender *) sender;

	dbg(5, "asn_loop_send_udp: %s", line);

	sendto(s->fd, line, strlen(line), 0, (struct sockaddr *) &(s->addr), sizeof(struct sockaddr_in));
}
