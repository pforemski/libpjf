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

#ifndef _SELECT_H
#define _SELECT_H

/* According to POSIX.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"

/** Waits until one or more of file descriptors are ready for reading
 * @param fdlist      list of FDs to monitor - a thash indexed by (unsigned int) FDs holding arbitrary (void *)
 *                    XXX: a thash because its easier to check/delete elements
 *                    XXX: use THASH_*_UINT() macros
 * @param timeout_ms  timeout, in miliseconds, after which we should exit, even if no FDs are ready
 *                    0 means exit immediately after polling, NULL means we can block
 *                    under Linux, value of timeout_ms is updated to reflect the amount of time left
 * @retval NULL       no result, repeat (e.g. select() interrupted)
 * @return            a NEW thash with elements REFERENCING elements of fdlist, for which FDs are ready for reading
 * @note              all monitored FDs must be > 0 */
thash *asn_rselect(thash *fdlist, uint32_t *timeout_ms, mmatic *mm);

/** Initialize generic main event loop */
void asn_loop_init(mmatic *mm);

/** Deinitialize main event loop, freeing the memory */
void asn_loop_deinit(void);

/** Start the main event loop */
void asn_loop(uint32_t timer);

/** Connect using TCP/IP
 * @param ipaddr   IPv4 address to connect to
 * @param port     TCP/IP port to use
 * @param cb       callback function to call each time new line is read */
FILE *asn_loop_connect_tcp(const char *ipaddr, const char *port, void (*cb)(const char *line));

#endif /* _SELECT_H */
