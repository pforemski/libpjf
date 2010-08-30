/*
 * fifos - fifo statistics
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

#ifndef _FIFOS_H_
#define _FIFOS_H_

#include "lib.h"

struct fifos_el {
	char *value;       /**> value to show */
	long wd;           /**> ID under inotify */
	int fd;            /**> fifo opened for r/w */
	int state;         /**> what were currently waiting for */
};

struct fifos {
	char *dir;         /**> directory for fifos */

	int fd;            /**> inotify fd */
	thash *data;       /**> current state "on disk", name => struct fifos_el */
	thash *wd2n;       /**> for inotify, wd => name */

	mmatic *mm;        /**> our memory */
};

/** Inits fifos in given dir
 * @param  dir    directory to create fifos in */
struct fifos *fifos_init(const char *dir);

/** Deletes fifos instance
 * @param f  fifos instance data */
void fifos_deinit(struct fifos *f);

/** Updates state
 * Synchronizes internal last state info with the supplied one, creating/deleting fifos under dir selected in
 * fifos_init(), copies values using mm
 * @param f      fifos instance data
 * @param state  a thash of name => value pairs
 * @return       a fd to monitor for read(2) to be handled by fifos_read() */
int fifos_update(struct fifos *f, thash *state);

/** Handle a read(2) request
 * @param f  fifos instance data */
void fifos_read(struct fifos *f);

#endif /* _FIFOS_H_ */
