/*
 * select - simpler interface to select(2)
 *
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
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

#include "lib.h"

struct asn_fd *asn_newfd(int fd, void *prv, mmatic *mm)
{
	struct asn_fd *afd = mmalloc(sizeof(struct asn_fd));
	afd->fd = fd;
	afd->prv = prv;
	return afd;
}

tlist *asn_rselect(tlist *fdlist, uint32_t *timeout_ms, mmatic *mm)
{
	int r, nfds = -1;
	struct timeval tv;
	struct asn_fd *afd;
	tlist *ret;
	fd_set fds;

	FD_ZERO(&fds);

	tlist_reset(fdlist);
	while ((afd = tlist_iter(fdlist))) {
		if (afd->fd < 0) continue;

		FD_SET(afd->fd, &fds);
		nfds = MAX(nfds, afd->fd);
	}

	/* no proper input, no output */
	if (nfds < 0) return MMTLIST_CREATE(NULL);

	if (timeout_ms) {
		tv.tv_sec = *timeout_ms / 1000;
		tv.tv_usec = (*timeout_ms % 1000) * 1000;
	}

	dbg(8, "asn_rselect(): calling select() nfds=%d, timeout=%d\n", nfds, *timeout_ms);
	if ((r = select(nfds + 1, &fds, NULL, NULL, (timeout_ms) ? &tv: NULL) < 0)) {
		switch (errno) {
			case EBADF:  dbg(0, "asn_rselect(): unexpected EBADF received\n"); break;
			case EINTR:  break; /* signal handled, reload fds as they may have changed */
			case EINVAL: dbg(0, "asn_rselect(): unexpected EINVAL received\n"); break;
			case ENOMEM: die("asn_rselect(): out of memory\n");
			default:     dbg(0, "asn_rselect(): %m\n"); break;
		}

		return NULL;
	}

	ret = MMTLIST_CREATE(NULL);

	tlist_reset(fdlist);
	while ((afd = tlist_iter(fdlist))) {
		if (afd->fd < 0) continue;

		if (FD_ISSET(afd->fd, &fds)) {
			dbg(8, "asn_rselect(): fd %d ready\n", afd->fd);
			tlist_push(ret, afd); /* magic! */
		}
	}

	if (timeout_ms)
		*timeout_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

	return ret;
}
