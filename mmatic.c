/*
 * mmatic - memory allocation manager, or a manual garbage collector
 *
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
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
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/mman.h>

#include "misc.h"
#include "mmatic.h"

#define PTR(ptr) ((uint8_t *) ptr)

mmatic *mmatic_create(void)
{
	mmatic *mgr;

	mgr = asn_malloc(sizeof(mmatic));
	mgr->totalloc = 0;
	mgr->first = mgr->last = asn_malloc(sizeof(mmchunk));
	bzero(mgr->first, sizeof(mmchunk));

	return mgr;
}

void mmatic_free(mmatic *mgr)
{
	mmchunk *chunk, *nchunk;

	dbg(10, "mmatic_free(%p): freeing\n", mgr);

	chunk = mgr->first;
	while (chunk) {
		nchunk = chunk->next;
		if (chunk->shared)
			munmap(chunk, chunk->alloc + sizeof(mmchunk));
		else
			free(chunk);
		chunk = nchunk;
	}

	free(mgr);
	mgr = 0;
}

void mmatic_freeptr(void *mem)
{
	mmchunk *chunk = (mmchunk *) (PTR(mem) - sizeof(mmchunk)); /* XXX */

	chunk->prev->next = chunk->next;
	if (chunk->next)
		chunk->next->prev = chunk->prev;
	else /* (mgr->last == chunk) */
		chunk->mgr->last = chunk->prev;

	chunk->mgr->totalloc -= chunk->alloc;

	if (chunk->shared)
		munmap(chunk, chunk->alloc + sizeof(mmchunk));
	else
		free(chunk);

	mem = 0;
}

int mmatic_isof(void *mem, mmatic *mm)
{
	mmchunk *chunk = (mmchunk *) (PTR(mem) - sizeof(mmchunk)); /* XXX */
	return (chunk->mgr == mm);
}

void mmatic_summary(mmatic *mgr, int dbglevel)
{
	mmchunk *chunk;

	dbg(dbglevel, "--- MMATIC MEMORY SUMMARY START (%p) ---\n", mgr);
	dbg(dbglevel, "--- total memory allocated: %u bytes\n", mgr->totalloc);

	if (mgr->first) {
		chunk = mgr->first->next;
		while (chunk) {
			dbg(dbglevel, "  %p: %uB for %s:%u\n",
			    PTR(chunk) + sizeof(mmchunk), chunk->alloc, chunk->cfile, chunk->cline);
			chunk = chunk->next;
		}
	}

	dbg(dbglevel, "--- MMATIC MEMORY SUMMARY END (%p) ---\n", mgr);
}

void *mmatic_realloc(void *mem, size_t size, mmatic *mgr)
{
	mmchunk *chunk = (mmchunk *) (PTR(mem) - sizeof(mmchunk)); /* XXX */
	void *newmem;

	newmem = mmatic_allocate(chunk->shared, size, mgr, chunk->cfile, chunk->cline);
	memcpy(newmem, mem, chunk->alloc);
	mmatic_freeptr(mem);

	return newmem;
}

void *asn_malloc(size_t size)
{
	void *mem;

	dbg(11, "tmalloc(%u)\n", size);

	mem = malloc(size);
	if (!mem)
		die("malloc(%u) failed, out of memory\n", (unsigned int) size);

	return mem;
}

/* now then, that's a handy tool! */
char *mmatic_printf(mmatic *mm, const char *fmt, ...)
{
	va_list args; char *buf;
	va_start(args, fmt); buf = mmalloc(1024); vsnprintf(buf, 1024, fmt, args); va_end(args);
	return buf;
}

/* it seems its easier to have code duplication than coping with va_lists */
char *tmalloc_printf(const char *fmt, ...)
{
	va_list args; char *buf;
	va_start(args, fmt); buf = asn_malloc(1024); vsnprintf(buf, 1024, fmt, args); va_end(args);
	return buf;
}
