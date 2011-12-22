/*
 * mmatic - memory allocation manager, or a manual garbage collector
 *
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
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

/** Return size of allocated memory */
#define mmatic_size(mgr) ((mgr)->totalloc)

/** mmatic memory allocator
 * @param size        amount of memory to allocate (bytes)
 * @param mgr_or_mem  memory manager or any variable allocated in mmatic
 * @param cfile       C source code file
 * @param cline       C source code line
 * @note the start and flags arguments are ignored when shared is 0 */
void *_mmatic_alloc(void *mgr_or_mem, size_t size, const char *cfile, unsigned int cline);
#define mmatic_alloc(mgr, size)  _mmatic_alloc(((void *) mgr), (size), __FILE__, __LINE__)

/** Wrapper around _mmatic_alloc() which sets memory to 0 */
void *_mmatic_zalloc(void *mgr_or_mem, size_t size, const char *cfile, unsigned int cline);
#define mmatic_zalloc(mgr, size) _mmatic_zalloc(((void *) mgr), (size), __FILE__, __LINE__)

/** Reallocate memory, possibly changing manager and/or size
 * @param mem           memory address
 * @param size          new size
 * @param mgr_or_mem    may be NULL = no manager change
 * @return              copy of mem */
void *_mmatic_realloc(void *mem, size_t size, void *mgr_or_mem, const char *cfile, unsigned int cline);

/** Allocate bigger chunk and copy contents
 * @param mem   already allocated memory to be moved
 * @param size  new size
 * @return      copy of mem */
#define mmatic_resize(mem, size) _mmatic_realloc((mem), (size), NULL, __FILE__, __LINE__)

/** Move chunk to another mgr
 * @param mem    already allocated memory to be moved
 * @param newmgr new manager
 * @return       copy of mem */
#define mmatic_moveto(mem, newmgr) _mmatic_realloc((mem), 0, ((void *) newmgr), __FILE__, __LINE__)

/** Clone memory
 * @param mem   memory to clone
 * @param mm    optional new mmatic
 * @return      copy of mem */
void *_mmatic_copy(const void *mem, void *mm, const char *cfile, unsigned int cline);

/** Copy memory to new memory
 * @return   copy of mem */
#define mmatic_copyto(mem, newmgr) _mmatic_copy((mem), ((void *) newmgr), __FILE__, __LINE__)

/** Clone memory in same memory
 * @return   copy of mem */
#define mmatic_copy(mem) _mmatic_copy((mem), NULL, __FILE__, __LINE__)

/*****************************************************************************/

/** Frees all memory and destroys given manager
 * @param mgr_or_mem    memory manager or memory (see _mmatic_alloc()) */
void mmatic_destroy_(void *mgr_or_mem, const char *cfile, unsigned int cline);
#define mmatic_destroy(a) mmatic_destroy_((a), __FILE__, __LINE__)

/** Frees one specific pointer
 * @param mem       memory from mmatic_alloc() */
void mmatic_free(const void *memptr);

/*****************************************************************************/

/** Print memory usage summary */
void mmatic_summary(mmatic *mgr, int dbglevel);

/** strdup() using mmatic_alloc
 * @param s         string to duplicate
 * @param cfile     C source code file
 * @param cline     C source code line */
char *_mmatic_strdup(void *mgr, const char *s, const char *cfile, unsigned int cline);
#define mmatic_strdup(mgr, str) _mmatic_strdup(((void *) mgr), (str), __FILE__, __LINE__)

/** An in-place sprintf()
 * @return allocated buffer, filled using sprintf()
 */
char *mmatic_sprintf(void *mgr, const char *fmt, ...);

#endif /* _MMATIC_H_ */
