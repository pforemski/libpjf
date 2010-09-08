/*
 * mmatic - memory allocation manager, or a manual garbage collector
 *
 * This file is part of libasn
 * Copyright (C) 2005-2010 ASN Sp. z o.o.
 * Author: Pawel Foremski <pforemski@asn.pl>
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
#include <stdbool.h>

struct mmatic;

typedef struct mmchunk {
	uint32_t tag;              /** For sanity checks */
	struct mmatic *mgr;        /** Manager */
	struct mmchunk *next;      /** Next chunk */
	struct mmchunk *prev;      /** Previus chunk */
	const char *cfile;         /** Source code file which requested allocation */
	unsigned int cline;        /** Source code line which requested allocation */
	unsigned long alloc;       /** Number of bytes allocated for this chunk */
	bool shared;               /** Allocation type */
} mmchunk;

typedef struct mmatic {
	uint32_t tag;              /** For sanity checks */
	mmchunk *first;            /** First chunk */
	mmchunk *last;             /** Last chunk */
	unsigned int totalloc;     /** Total allocation */
} mmatic;

/*****************************************************************************/

/** Creates new mmatic object */
void *mmatic_create(void);

/** mmatic memory allocator
 * @param size        amount of memory to allocate (bytes)
 * @param mgr_or_mem  memory manager or any variable allocated in mmatic
 * @param zero        set memory to 0 after allocation
 * @param shared      use mmap() and make memory writable after fork()
 * @param start       memory start address for mmap() to use
 * @param flags       additional flags for mmap()
 * @param cfile       C source code file
 * @param cline       C source code line
 * @note the start and flags arguments are ignored when shared is 0 */
void *mmatic_allocate(size_t size, void *mgr_or_mem, bool zero, bool shared, void *start, int flags,
	const char *cfile, unsigned int cline);

#define mmatic_alloc(size, mgr)   mmatic_allocate((size), ((void *) mgr), 0, 0, NULL, 0, __FILE__, __LINE__)
#define mmatic_shalloc(size, mgr) mmatic_allocate((size), ((void *) mgr), 0, 1, NULL, 0, __FILE__, __LINE__)
#define mmalloc(size)   mmatic_alloc((size), mm)
#define mmshalloc(size) mmatic_shalloc((size), mm)

#define mmatic_zalloc(size, mgr)   mmatic_allocate((size), ((void *) mgr), 1, 0, NULL, 0, __FILE__, __LINE__)
#define mmatic_shzalloc(size, mgr) mmatic_allocate((size), ((void *) mgr), 1, 1, NULL, 0, __FILE__, __LINE__)
#define mmzalloc(size)   mmatic_zalloc((size), mm)
#define mmzshalloc(size) mmatic_zshalloc((size), mm)

/** Reallocate memory, possibly changing manager and/or size
 * @param mgr_or_mem    see mmatic_allocate(), may be NULL = no manager change */
void *mmatic_realloc_(void *mem, size_t size, void *mgr_or_mem, const char *cfile, unsigned int cline);

/** Allocate bigger chunk and copy contents
 * @param mem   already allocated memory to be moved
 * @param size  new size */
#define mmatic_realloc(mem, size) mmatic_realloc_((mem), (size), NULL, __FILE__, __LINE__)

/** Move chunk to another mgr
 * @param mem    already allocated memory to be moved
 * @param newmgr new mgr */
#define mmatic_move(mem, newmgr) mmatic_realloc_((mem), 0, ((void *) newmgr), __FILE__, __LINE__)

/*****************************************************************************/

/** Frees all memory and destroys given manager
 * @param mgr_or_mem    memory manager or memory (see mmatic_allocate())
 * @note sets *mgr = 0 */
void mmatic_free_(void **mgr_or_mem, const char *cfile, unsigned int cline);
#define mmatic_free(a) mmatic_free_((void **) &(a), __FILE__, __LINE__)
#define mmfree() mmatic_free(mm, __FILE__, __LINE__)

/** Frees one specific pointer
 * @param mem       memory from mmatic_alloc()
 * @note set *mem = 0 */
void mmatic_freeptr_(void **mem);
#define mmatic_freeptr(a) mmatic_freeptr_((void **) &(a))

/** A counterpart to mmatic_freeptr which doesnt do mem=0 */
void mmatic_freeptrs(void *ptr);
#define mmfreeptr mmatic_freeptrs

/*****************************************************************************/

/** Print memory usage summary */
void mmatic_summary(mmatic *mgr, int dbglevel);
#define mmsummary(lvl) (mmatic_summary(mm, (lvl)))

/** strdup() using mmatic_alloc
 * @param s         string to duplicate
 * @param cfile     C source code file
 * @param cline     C source code line
 */
static inline char *_mmatic_strdup(const char *s, void *mgr, const char *cfile, unsigned int cline)
{
	char *newm;
	newm = mmatic_allocate(strlen(s) + 1, mgr, 0, 0, NULL, 0, cfile, cline);
	strcpy(newm, s);
	return newm;
}
#define mmatic_strdup(str, mgr) (_mmatic_strdup(str, (void *) mgr, __FILE__, __LINE__))
#define mmstrdup(str) (mmatic_strdup((str), mm))

/** An in-place snprintf()
 * @return allocated buffer, filled using snprintf()
 */
char *mmatic_printf_(void *mm, const char *fmt, ...);
#define mmatic_printf(mm, ...) mmatic_printf_((void *) mm, __VA_ARGS__)
#define mmprintf(...) mmatic_printf(mm, __VA_ARGS__)

/** Generic struct maker: allocates memory for given struct and fills it with given data */
#define mmatic_make(mgr, type, ...) memcpy(mmatic_alloc(sizeof(type), (mgr)), &(type){ __VA_ARGS__ }, sizeof(type))

/** A wrapper around mmatic_make() which uses global "mm" object */
#define mmake(type, ...) mmatic_make(mm, type, __VA_ARGS__)

#endif /* _MMATIC_H_ */
