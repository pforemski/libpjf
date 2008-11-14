/*
 * fifos - fifo statistics
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/inotify.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include "fifos.h"
#include "misc.h"
#include "thash.h"
#include "mmatic.h"

struct fifos *fifos_init(const char *dir)
{
	mmatic *mm = mmatic_create();
	struct fifos *f = mmalloc(sizeof(struct fifos));
	int i, fd;
	struct dirent **ls;
	char *p;

	f->dir   = asn_abspath(dir, mm);
	f->data  = MMTHASH_CREATE_STR(mmfreeptr);
	f->fd    = inotify_init();
	f->wd2n  = MMTHASH_CREATE_PTR(mmfreeptr); /* strings indexed by integers (equiv. to pointers) */
	f->mm    = mm;

	if (asn_isdir(f->dir) == -1) {
		asn_mkdir(f->dir, f->mm, NULL); /* XXX: dont care about errors */
	}
	else {
		i = scandir(f->dir, &ls, 0, alphasort);
		if (i < 0) { dbg(1, "fifos_init(%s): %m\n", f->dir); return f; }

		while (i--) {
			p = tmprintf("%s/%s", f->dir, ls[i]->d_name);
			if (asn_isfifo(p) > 0) {
				fd = open(p, O_RDWR); /* in case someone is waiting */
				unlink(p);
				if (fd > 0) close(fd);
			}
			free(p);
			free(ls[i]);
		}

		free(ls);
	}

	dbg(9, "fifos_init(): initialized inotify with fd=%d\n", f->fd);

	return f;
}

void fifos_deinit(struct fifos *f)
{
	const char *n;

	/* deinits inotify */
	close(f->fd);

	thash_reset(f->data);
	while (thash_iter(f->data, &n)) {
		dbg(8, "fifos_deinit(): removing %s\n", n);
		unlink(mmatic_printf(f->mm, "%s/%s", f->dir, n));
	}

	mmatic_free(f->mm);
	f = 0;
}

int fifos_update(struct fifos *f, thash *state)
{
	char *n, *v, *p;
	int i, fd;
	uint32_t wd;
	mmatic *mm = mmatic_create(); /* temp. memory */
	struct fifos_el *data;

	/* handle new */
	thash_reset(state);
	while ((v = thash_iter(state, &n))) {
		if ((data = thash_get(f->data, n))) { /* just update the value */
			mmatic_freeptr(data->value);
			data->value = mmatic_strdup(v, f->mm);
		}
		else {
			p = mmprintf("%s/%s", f->dir, n);
			if (mkfifo(p, 0666) < 0) { dbg(1, "mkfifo(%s): %m\n", p); continue; }

			fd = open(p, O_RDWR); /* XXX: r/w so open(O_RDONLY) wont block */
			if (fd < 0) { dbg(1, "open(%s): %m\n", p); continue; }

			wd = inotify_add_watch(f->fd, p, IN_OPEN | IN_CLOSE);
			if (wd < 0) { dbg(1, "inotify_add_watch(%s): %m\n", p); continue; }

			data         = mmatic_alloc(sizeof(*data), f->mm);
			data->value  = mmatic_strdup(v, f->mm);  /* XXX: copy the value */
			data->wd     = wd;
			data->fd     = fd;
			data->state  = 1;

			thash_set(f->data, n, data);
			thash_set(f->wd2n, (void *) wd, mmatic_strdup(n, f->mm));

			dbg(8, "fifos_update(): added watch on %s\n", p);
		}
	}

	/* handle old */
	thash_reset(f->data);
	while ((data = thash_iter(f->data, &n))) {
		if (thash_get(state, n)) continue; /* yeap, still there */

		i = inotify_rm_watch(f->fd, data->wd);
		if (i < 0) { dbg(1, "inotify_rm_watch(%s): %m\n", p); continue; }

		close(data->fd);

		p = mmprintf("%s/%s", f->dir, n);
		if (unlink(p) < 0) { dbg(1, "unlink(%s): %m\n", p); continue; }

		dbg(8, "fifos_update(): deleting watch on %s\n", p);
		thash_set(f->wd2n, (void *) data->wd, NULL);

		mmfreeptr(data->value);
		thash_set(f->data, n,  NULL);
	}

	mmfree();
	return f->fd;
}

void fifos_read(struct fifos *f)
{
	int i;
	struct inotify_event e; /* XXX: we shouldnt get any e.len > 0 */
	char *n, *p, *v = NULL;
	struct fifos_el *data;

	i = read(f->fd, &e, sizeof(e));
	if (i < 0) { dbg(1, "fifos_read(): read(%d): %m\n", f->fd); return; }
	else if (i != sizeof(e)) { dbg(1, "fifos_read(): read invalid number of bytes (%d)\n", i); return; }

	dbg(5, "fifos_read(): e.mask=%d\n", e.mask);

	n = thash_get(f->wd2n, (void *) e.wd);
	if (!n) { dbg(e.mask == IN_IGNORED ? 9 : 0, "fifos_read(): unknown wd %d\n", e.wd); return; }

	data = thash_get(f->data, n);
	if (!data) die("fifos_read(): no data for '%s' (wd %d)\n", n, e.wd);

	p = tmprintf("%s/%s", f->dir, n);
	switch (e.mask) {
		case IN_OPEN:
			dbg(11, "fifos_read(): IN_OPEN\n");
			if (data->state == 4) { data->state = 1; goto fr_end; }
			else if (data->state != 1) goto fr_end;

			v = tmprintf("%s", data->value);
			i = write(data->fd, v, strlen(v));
			if (i < 0) dbg(0, "fifos_read(): write(): %m\n");

			data->state = 2;
			close(data->fd);
			break;
		case IN_CLOSE_WRITE:
		case IN_CLOSE_NOWRITE:
			dbg(11, "fifos_read(): IN_CLOSE\n");
			if (data->state == 2) { data->state = 3; goto fr_end; }
			else if (data->state != 3) goto fr_end;

			data->state = 4;
			data->fd = open(p, O_RDWR);
			if (data->fd < 0) { dbg(0, "open(%s): %m\n", p); goto fr_end; }
			break;
	}

fr_end:
	if (v) free(v);
	free(p);
	return;
}
