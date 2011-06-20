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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MMATIC_H_
#define _MMATIC_H_

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
 * @param cfile       C source code file
 * @param cline       C source code line
 * @note the start and flags arguments are ignored when shared is 0 */
void *mmatic_allocate(void *mgr_or_mem, size_t size, const char *cfile, unsigned int cline);
#define mmatic_alloc(mgr, size)  mmatic_allocate(((void *) mgr), (size), __FILE__, __LINE__)
#define mmalloc(size)            mmatic_allocate(mm, (size), __FILE__, __LINE__)

/** Wrapper around mmatic_allocate() which sets memory to 0 */
void *mmatic_zallocate(void *mgr_or_mem, size_t size, const char *cfile, unsigned int cline);
#define mmatic_zalloc(mgr, size) mmatic_zallocate(((void *) mgr), (size), __FILE__, __LINE__)
#define mmzalloc(size)           mmatic_zallocate(mm, (size), __FILE__, __LINE__)

/** Reallocate memory, possibly changing manager and/or size
 * @param mem           memory address
 * @param size          new size
 * @param mgr_or_mem    may be NULL = no manager change
 * @return              copy of mem */
void *mmatic_reallocate(void *mem, size_t size, void *mgr_or_mem, const char *cfile, unsigned int cline);

/** Allocate bigger chunk and copy contents
 * @param mem   already allocated memory to be moved
 * @param size  new size
 * @return      copy of mem */
#define mmatic_resize(mem, size) mmatic_reallocate((mem), (size), NULL, __FILE__, __LINE__)

/** Move chunk to another mgr
 * @param mem    already allocated memory to be moved
 * @param newmgr new manager
 * @return       copy of mem */
#define mmatic_move(mem, newmgr) mmatic_reallocate((mem), 0, ((void *) newmgr), __FILE__, __LINE__)

/** Clone memory
 * @param mem   memory to clone
 * @param mm    optional new mmatic
 * @return      copy of mem */
void *mmatic_clone_(const void *mem, void *mm, const char *cfile, unsigned int cline);

/** Copy memory to new memory
 * @return   copy of mem */
#define mmatic_copyto(mem, newmgr) mmatic_clone_((mem), ((void *) newmgr), __FILE__, __LINE__)

/** Clone memory in same memory
 * @return   copy of mem */
#define mmatic_clone(mem) mmatic_clone_((mem), NULL, __FILE__, __LINE__)

/*****************************************************************************/

/** Frees all memory and destroys given manager
 * @param mgr_or_mem    memory manager or memory (see mmatic_allocate()) */
void mmatic_free_(void *mgr_or_mem, const char *cfile, unsigned int cline);
#define mmatic_free(a) mmatic_free_((a), __FILE__, __LINE__)
#define mmfree() mmatic_free(mm)

/** Frees one specific pointer
 * @param mem       memory from mmatic_alloc() */
void mmatic_freeptr(const void *memptr);
#define mmfreeptr mmatic_freeptr

/*****************************************************************************/

/** Print memory usage summary */
void mmatic_summary(mmatic *mgr, int dbglevel);
#define mmsummary(lvl) (mmatic_summary(mm, (lvl)))

/** strdup() using mmatic_alloc
 * @param s         string to duplicate
 * @param cfile     C source code file
 * @param cline     C source code line */
char *mmatic_strdup_(void *mgr, const char *s, const char *cfile, unsigned int cline);
#define mmatic_strdup(mgr, str) mmatic_strdup_(((void *) mgr), (str), __FILE__, __LINE__)
#define mmstrdup(str)           mmatic_strdup_(mm, (str), __FILE__, __LINE__)

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
