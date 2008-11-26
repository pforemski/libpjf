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

thash *asn_rselect(thash *fdlist, uint32_t *timeout_ms, mmatic *mm)
{
	unsigned int fd, nfds = 0;
	struct timeval tv;
	thash *ret;
	fd_set fds;
	void *prv;

	FD_ZERO(&fds);

	thash_reset(fdlist);
	while ((prv = THASH_ITER_UINT(fdlist, &fd))) {
		if (fd >= FD_SETSIZE) {
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
	if (select(nfds + 1, &fds, NULL, NULL, (timeout_ms) ? &tv: NULL) < 0) {
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
