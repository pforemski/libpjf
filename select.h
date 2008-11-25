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

#ifndef _SELECT_H
#define _SELECT_H

/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"

struct asn_fd {
	int   fd;    /**> file descriptor */
	void *prv;   /**> user data */
};

/** Quick struct asn_fd creator */
struct asn_fd *asn_newfd(int fd, void *prv, mmatic *mm);

/** Waits until a file descriptor is ready for reading
 * @param fdlist      list of FDs to monitor - a tlist of (struct asn_fd *)
 * @param timeout_ms  timeout, in miliseconds, after which we should exit, even if no FDs are ready
 *                    0 means exit immediately after polling, NULL means we can block
 *                    under Linux, value of timeout_ms is updated to reflect the amount of time left
 * @retval NULL       no result, repeat (e.g. select() interrupted)
 * @return            a NEW tlist with elements REFERENCING elements of fdlist, for which FDs are ready for reading */
tlist *asn_rselect(tlist *fdlist, uint32_t *timeout_ms, mmatic *mm);

#endif /* _SELECT_H */
