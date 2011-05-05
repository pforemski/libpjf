/*
 * mmatic - memory allocation manager, or a manual garbage collector
 *
 * This file is part of libpjf
 * Copyright (C) 2005-2010 ASN Sp. z o.o.
 * Author: Pawel Foremski <pawel@foremski.pl>
 *
 * libpjf is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * libpjf is distributed in the hope that it will be useful, but WITHOUT ANY
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
#include "lib.h"

/** Memory tag for type detection */
#define TAG_MGR   0xBABBA777

/** Memory tag for type detection */
#define TAG_CHUNK 0xABBA1234

/** Get mmatic memory chunk out of a pointer */
#define PTR_TO_CHUNK(ptr)   ((ptr) ? (mmchunk *) (((uint8_t *) ptr) - sizeof(mmchunk)) : NULL)

/** Get mmatic memory manager out of a chunk */
#define CHUNK_TO_PTR(chunk) ((chunk) ? (void *)  (((uint8_t *) chunk) + sizeof(mmchunk)) : NULL)

/** Verify chunk tag if run in debug mode */
#define IS_CHUNK(chunk)  ((chunk) && (chunk)->tag == TAG_CHUNK)

/** Verify manager tag if run in debug mode */
#define IS_MGR(mgr)      ((mgr) && (mgr)->tag == TAG_MGR)


/*****************************************************************************/
/***************************** Allocations ***********************************/
/*****************************************************************************/

#define ALLOC(ptr, size) if (!(ptr = malloc(size))) die("Out of memory");
void *mmatic_create(void)
{
	mmatic *mgr;

	ALLOC(mgr, sizeof(mmatic));
	mgr->tag = TAG_MGR;
	mgr->totalloc = 0;

	ALLOC(mgr->first, sizeof(mmchunk));
	mgr->last = mgr->first;
	bzero(mgr->first, sizeof(mmchunk));
	mgr->first->mgr = mgr;

	return mgr;
}

void *mmatic_allocate(void *mgr_or_mem, size_t size, const char *cfile, unsigned int cline)
{
	mmatic *mgr;
	mmchunk *chunk;

	mgr = mgr_or_mem;
	if (!IS_MGR(mgr)) {
		chunk = PTR_TO_CHUNK(mgr_or_mem);

		if (IS_CHUNK(chunk))
			mgr = chunk->mgr;
		else
			die("Requested allocation in invalid space (called from %s:%u)", cfile, cline);
	}

	chunk = malloc((sizeof *chunk) + size);
	if (!chunk)
		die("Out of memory (called from %s:%u)", cfile, cline);

	chunk->tag      = TAG_CHUNK;
	chunk->alloc    = size;
	chunk->cfile    = cfile;
	chunk->cline    = cline;
	chunk->next     = NULL;
	chunk->prev     = mgr->last;
	chunk->mgr      = mgr;
	mgr->last->next = chunk;
	mgr->last       = chunk;
	mgr->totalloc  += size;

	return CHUNK_TO_PTR(chunk);
}

void *mmatic_zallocate(void *mgr, size_t size, const char *cfile, unsigned int cline)
{
	void *ptr;
	ptr = mmatic_allocate(mgr, size, cfile, cline);
	return ptr ? memset(ptr, 0, size) : ptr;
}

void *mmatic_reallocate(void *mem, size_t size, void *mgr_or_mem, const char *cfile, unsigned int cline)
{
	mmchunk *chunk;
	void *newmem;

	chunk = PTR_TO_CHUNK(mem);
	pjf_assert(IS_CHUNK(chunk));

	if (!mgr_or_mem)
		mgr_or_mem = (void *) chunk->mgr;

	if (!size)
		size = chunk->alloc;

	newmem = mmatic_allocate(mgr_or_mem, size, cfile, cline);
	memcpy(newmem, mem, chunk->alloc);
	mmatic_freeptr(mem);

	return (mem = newmem);
}

void *mmatic_clone_(const void *mem, void *mm, const char *cfile, unsigned int cline)
{
	mmchunk *chunk;
	void *newmem;

	chunk = PTR_TO_CHUNK(mem);
	pjf_assert(IS_CHUNK(chunk));

	newmem = mmatic_allocate(mm ? mm : chunk->mgr, chunk->alloc, cfile, cline);
	memcpy(newmem, mem, chunk->alloc);

	return newmem;
}

/*****************************************************************************/
/************************** Free functions ***********************************/
/*****************************************************************************/

void mmatic_free_(void **mgr_or_mem, const char *cfile, unsigned int cline)
{
	mmatic *mgr = *mgr_or_mem;
	mmchunk *chunk, *nchunk;

	if (!IS_MGR(mgr)) {
		chunk = PTR_TO_CHUNK(*mgr_or_mem);

		if (IS_CHUNK(chunk))
			mgr = chunk->mgr;
		else
			die("Requested deallocation of invalid space (called from %s:%u)", cfile, cline);
	}

	pjf_assert(IS_MGR(mgr));
	dbg(12, "%p: freeing\n", mgr);

	chunk = mgr->first;
	while (chunk) {
		nchunk = chunk->next;
		free(chunk);
		chunk = nchunk;
	}

	free(mgr);
	*mgr_or_mem = 0;
}

void mmatic_freeptr_(void **memptr)
{
	void *mem = *memptr;
	mmchunk *chunk = PTR_TO_CHUNK(mem);

	pjf_assert(IS_CHUNK(chunk));

	chunk->prev->next = chunk->next;
	if (chunk->next)
		chunk->next->prev = chunk->prev;
	else /* XXX: implicit: (mgr->last == chunk) */
		chunk->mgr->last = chunk->prev;

	chunk->mgr->totalloc -= chunk->alloc;
	free(chunk);

	*memptr = 0;
}

void mmatic_freeptrs(void *ptr)
{
	mmatic_freeptr_((void **) &ptr);
}

/*****************************************************************************/
/****************************** Utilities ************************************/
/*****************************************************************************/

char *mmatic_strdup_(void *mgr, const char *s, const char *cfile, unsigned int cline)
{
	char *newm;

	if (!s) return NULL;

	newm = mmatic_allocate(mgr, strlen(s) + 1, cfile, cline);
	strcpy(newm, s);

	return newm;
}

void mmatic_summary(mmatic *mgr, int dbglevel)
{
	mmchunk *chunk;

	dbg(dbglevel, "--- MMATIC MEMORY SUMMARY START (%p) ---\n", mgr);
	dbg(dbglevel, "--- total memory allocated: %u bytes\n", mgr->totalloc);

	if (mgr->first) {
		chunk = mgr->first->next;
		while (chunk) {
			dbg(dbglevel, "  %p: %uB for %s:%u\n", CHUNK_TO_PTR(chunk), chunk->alloc, chunk->cfile, chunk->cline);
			chunk = chunk->next;
		}
	}

	dbg(dbglevel, "--- MMATIC MEMORY SUMMARY END (%p) ---\n", mgr);
}

/* now then, that's a handy tool! */
char *mmatic_printf_(void *mm, const char *fmt, ...)
{
	va_list args; char *buf;

	va_start(args, fmt);
	buf = mmalloc(BUFSIZ);
	vsnprintf(buf, BUFSIZ, fmt, args);
	va_end(args);

	return buf;
}
