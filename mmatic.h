/*
 * mmatic - memory allocation manager, or a manual garbage collector
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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MMATIC_H_
#define _MMATIC_H_

#define __USE_MISC 1 /* for MAP_ANONYMOUS */

#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

struct mmatic;

typedef struct mmchunk {
	/** Allocation type */
	bool shared;

	/** Number of bytes allocated for this chunk */
	unsigned int alloc;

	/** Source code file which requested allocation */
	const char *cfile;

	/** Source code line which requested allocation */
	unsigned int cline;

	/** Next chunk */
	struct mmchunk *next;

	/** Previus chunk */
	struct mmchunk *prev;

	/** Manager */
	struct mmatic *mgr;
} mmchunk;

typedef struct mmatic {
	/** First chunk */
	mmchunk *first;

	/** Last chunk */
	mmchunk *last;

	/** Total allocation */
	unsigned int totalloc;
} mmatic;

/*
 * Functions
 */
/** Creates new mmatic object */
mmatic *mmatic_create(void);

/** Hacky shortcut to initialize a new mmatic under the default symbol name */
#define MMNEW() mmatic *mm = mmatic_create()

/** A shortcut for freeing the default mm */
#define MMEND() mmatic_free(mm)

/** mmatic memory allocator
 * @remark it sits in *.h because its inline
 * @param shared    use mmap() and make memory writable after fork()
 * @param size      amount of memory to allocate (bytes)
 * @param mgr       memory manager
 * @param start     memory start address for mmap() to use
 * @param flags     additional flags for mmap()
 * @param cfile     C source code file
 * @param cline     C source code line
 * @note the start and flags arguments are ignored when shared is 0
 */
static inline void *mmatic_allocate(bool shared, size_t size, mmatic *mgr, void *start, int flags, const char *cfile, unsigned int cline)
{
	mmchunk *chunk;

	chunk = (shared) ?
		mmap(start, sizeof(mmchunk) + size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS|flags, 0, 0) :
		malloc(sizeof(mmchunk) + size);
	if (!chunk) _exit(150);

	chunk->shared   = shared;
	chunk->alloc    = size;
	chunk->cfile    = cfile;
	chunk->cline    = cline;
	chunk->next     = NULL;
	chunk->prev     = mgr->last;
	chunk->mgr      = mgr;
	mgr->last->next = chunk;
	mgr->last       = chunk;
	mgr->totalloc  += size;

	return (((uint8_t *) chunk) + sizeof(mmchunk)); /* XXX */
}

#define mmatic_alloc(size, mgr)   (mmatic_allocate(0, (size), (mgr), NULL, 0, __FILE__, __LINE__))
#define mmatic_shalloc(size, mgr) (mmatic_allocate(1, (size), (mgr), NULL, 0, __FILE__, __LINE__))
#define mmalloc(size)   (mmatic_allocate(0, (size), mm, NULL, 0, __FILE__, __LINE__))
#define mmshalloc(size) (mmatic_allocate(1, (size), mm, NULL, 0, __FILE__, __LINE__))

/** Frees all memory and destroys given manager
 * @param mgr       memory manager
 * @note sets *mgr = 0 */
void mmatic_free_(mmatic **mgr);
#define mmatic_free(a) (mmatic_free_(&(a)))
#define mmfree() (mmatic_free(mm))

/** Frees one specific pointer
 * @param mem       memory from mmatic_alloc()
 * @note set *mem = 0 */
void mmatic_freeptr_(void **mem);
#define mmatic_freeptr(a) (mmatic_freeptr_((void **) &(a)))

/** A counterpart to mmatic_freeptr which doesnt do mem=0 */
static inline void mmatic_freeptrs(void *ptr) { mmatic_freeptr_((void **) &ptr); }
#define mmfreeptr mmatic_freeptrs

/** Print memory usage summary */
void mmatic_summary(mmatic *mgr, int dbglevel);
#define mmsummary(lvl) (mmatic_summary(mm, (lvl)))

/** Allocate bigger chunk and copy contents
 * @param mem   already allocated memory to be moved
 * @param size  new size
 */
void *mmatic_realloc(void *mem, size_t size, mmatic *mgr);
#define mmrealloc(size, mem) (mmatic_realloc((mem), (size), mm))

/** Checks if ptr was allocated at given manager
 * @param mem ptr
 * @retval 1 yes
 * @retval 0 no
 */
int mmatic_isof(void *mem, mmatic *mm);

/*********************/
/***   Utilities   ***/
/*********************/

/** strdup() using mmatic_alloc
 * @param s         string to duplicate
 * @param cfile     C source code file
 * @param cline     C source code line
 */
static inline char *_mmatic_strdup(const char *s, mmatic *mgr, const char *cfile, unsigned int cline)
{
	char *newm;
	newm = mmatic_allocate(0, strlen(s) + 1, mgr, NULL, 0, cfile, cline);
	strcpy(newm, s);
	return newm;
}
#define mmatic_strdup(str, mgr) (_mmatic_strdup(str, mgr, __FILE__, __LINE__))
#define mmstrdup(str) (mmatic_strdup((str), mm))

/** Wrapper around malloc() which prints an error message and exits() with 1 in
 * case of an error
 *
 * @param size same as in stdlib's malloc()
 */
void *asn_malloc(size_t size);

/** An in-place snprintf()
 * @return allocated buffer, filled using snprintf()
 */
char *mmatic_printf(mmatic *mm, const char *fmt, ...);
#define mmprintf(...) (mmatic_printf(mm, __VA_ARGS__))

/** mmatic_printf() using asn_malloc() */
char *asn_malloc_printf(const char *fmt, ...);
#define tmprintf asn_malloc_printf

#endif /* _MMATIC_H_ */
