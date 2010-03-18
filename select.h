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

#define LOOP_OK        0x00
#define LOOP_EOF       0x01
#define LOOP_LOOPBACK  0x02

/** Callback function that uses line of text
 * @param line      line of text that caused the callback call
 * @param flags     see LOOP_*
 * @note do not reference line, its destroyed after callback exits */
typedef void (*loop_line_cb)(const char *line, int flags, void *prv);

/** Callback function that is executed on timeout */
typedef void (*loop_timeout_cb)(uint32_t delay, void *prv);

/** Waits until one or more of file descriptors are ready for reading
 * @param fdlist      list of FDs to monitor - a thash indexed by (unsigned int) FDs holding arbitrary (void *)
 *                    XXX: a thash because its easier to check/delete elements
 *                    XXX: use THASH_*_UINT() macros
 * @param timeout_ms  timeout, in miliseconds, after which we should exit, even if no FDs are ready
 *                    0 means exit immediately after polling, NULL means we can block
 *                    under Linux, value of timeout_ms is updated to reflect the amount of time left
 * @retval NULL       no result or no fd ready, repeat (e.g. select() interrupted)
 * @return            a NEW thash with elements REFERENCING elements of fdlist, for which FDs are ready for reading */
thash *asn_rselect(thash *fdlist, uint32_t *timeout_ms, mmatic *mm);

/** Initialize generic main event loop */
void asn_loop_init(void);

/** Free all memory */
void asn_loop_deinit(void);

/** Start the main event loop
 * @param timer   minimum time between two iterations [ms] */
void asn_loop(uint32_t timer);

/** Monitor given file descriptor
 * @param fd   file descriptor to monitor
 * @param cb   callback function to call each time new line is read */
void asn_loop_add_fd(int fd, loop_line_cb cb, void *prv);

/** Connect using TCP/IP
 * @param ipaddr   IPv4 address to connect to
 * @param port     TCP/IP port to use
 * @param cb       callback function to call each time new line is read
 * @return socket */
int asn_loop_connect_tcp(const char *ipaddr, const char *port, loop_line_cb cb, void *prv);

/** Create a UDP server
 * @param iface    interface to listen on (may be null)
 * @param ipaddr   IPv4 address to listen to (e.g. 0.0.0.0)
 * @param port     UDP port
 * @param cb       callback function to call each time new line is read
 * @return socket */
int asn_loop_listen_udp(const char *iface, const char *ipaddr, const char *port, loop_line_cb cb, void *prv);

/** Create a UDP sender
 * @param iface    interface to send packets on (may be null)
 * @param ipaddr   destination IP address
 * @param port     UDP port
 * @return opaque sender handle to pass to functions needing it */
void *asn_loop_udp_sender(const char *iface, const char *ipaddr, const char *port);

/** Send a line of text via UDP
 * @param sender   sender handle from asn_loop_udp_sender()
 * @param line     string to send (\0 is the ending character) */
void asn_loop_send_udp(void *sender, const char *line);

/** Schedule function call */
void asn_loop_schedule(struct timeval *when, loop_timeout_cb cb, void *prv);

/** Wrapper around asn_loop_schedule which accepts relative time
 * @param sec   number of seconds
 * @param usec  number of microseconds
 * @param cb    handler
 * @param prv   argument to pass to handler */
void asn_loop_schedule_in(uint32_t sec, uint32_t usec, loop_timeout_cb cb, void *prv);

#endif /* _SELECT_H */
