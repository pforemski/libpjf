/*
 * thash - trivial hash table
 *
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
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

#ifndef _THASH_H_
#define _THASH_H_

#include <stdlib.h>
#include <stdbool.h>

#include "mmatic.h"

/** Default size of new hash table. */
#define THASH_DEFAULT_SIZE 100

/** Maximal hash table usage before doubling it's size */
#define THASH_MAX_USAGE 0.60

/** Element of hash table */
typedef struct thash_el {
	/** Hash key used to access this field. */
	void *key;

	/** Element value. */
	void *val;

	/** Pointer at next element under this key.
	 * Used in case of key duplicates. */
	struct thash_el *next;
} thash_el;


/** A hash table */
typedef struct thash {
	/** Current size of hash table. */
	unsigned int size;

	/** Number of currently used entries ("slots"). */
	unsigned int used;

	/** The data. */
	thash_el **tbl;

	/** The hashing function. */
	unsigned int (*hash_func)(const void *key);

	/** The comparison function.
	 * @retval -1  key1 < key2
	 * @retval  0  key1 = key2
	 * @retval +1  key1 > key2
	 */
	int (*cmp_func)(const void *key1, const void *key2);

	/** Function used to free memory occupied by an element.
	 * @value NULL dont call me*/
	void (*free_func)(void *val);

	/** If 1, we operate on string keys only */
	bool strings_mode;

	/** mmatic */
	void *mm;

	/** Iterator: key number. */
	unsigned int counter_x;

	/** Iterator: element number in a list. */
	unsigned int counter_y;
} thash;

/** Creates a hashing table.
 *
 * @param  hash_func key hashing function to use; if NULL:
 *                     * if strings=1, a generic string hashing function will be used
 *                     * otherwise, we simply use the key directly
 * @param  cmp_func  key comparison function to use (in case of same indices); if NULL:
 *                     * if strings=1, strcmp() will be used
 *                     * otherwise, (a - b) will be used
 * @param  free_func function to use to free the value; if NULL, won't be freed
 * @param  strings   if true, *keys* will be copied and freed
 * @param  mm        mmatic; if NULL, use tmalloc
 * @retval NULL      an error occured
 */
thash *thash_create(unsigned int (*hash_func)(const void *key),
                    int (*cmp_func)(const void *key1, const void *key2),
                    void (*free_func)(), bool strings, void *mm);

/** Create a thash indexed by string, holding pointers to arbitrary data */
#define thash_create_strkey(ffn, mm) thash_create(NULL, NULL, (ffn), 1, (mm))

/** Create a thash indexed by pointers, holding pointers to arbitrary data */
#define thash_create_ptrkey(ffn, mm) thash_create(NULL, NULL, (ffn), 0, (mm))

/** Create a thash indexed by unsigned integers, holding pointers to arbitrary data */
#define thash_create_intkey thash_create_ptrkey

/** Frees a hash table.
 * @param hash the hash table
 */
void thash_free(thash *hash);

/** Flushes all elements from hash table.
 * @param hash the hash table
 */
void thash_flush(thash *hash);

/** Returns value of entry with given key.
 *
 * @param hash the hash table
 * @param key  the element key
 * @retval NULL entry does not exist */
void *thash_get(const thash *hash, const void *key);

/** A thash_get in case values are of unsigned int type */
#define thash_get_uint(a, b) ((unsigned long) thash_get((a), (b)))

/** A safe thash_get in case indices are of unsigned int type */
#define thash_uint_get(a, b) (thash_get((a), ((const void *) (unsigned long) b)))

/** Resets internal iteration counters.
 *
 * @param hash the hash table
 */
void thash_reset(thash *hash);

/** Iterate through table entries.
 *
 * @param  hash  hash table
 * @param  key   optional: memory to write pointer to key of entry being returned
 *               set to NULL if you don't want this
 * @return table entry
 * @retval NULL  end of table reached */
void *_thash_iter(thash *hash, void **key);
#define thash_iter(a, b) (_thash_iter((a), ((void **) (b))))

/** Safe iterator in case indices are of unsigned int type */
#define thash_iter_uint(a, b) (_thash_iter((a), ((void **) (unsigned long *) (b))))

#define thash_iter_loop(hash, k, v) thash_reset(hash); while (((v) = thash_iter((hash), &(k))))

/** Sets value of an element to given value.
 *
 * Resizes hash table if necessary.
 *
 * @param hash the hash table
 * @param key  the element key
 * @param val  element value; if NULL, then element is deleted
 * @remark     both key and val are referenced only
 */
void thash_set(thash *hash, const void *key, const void *val);

/** A thash_set in case value is of unsigned int type */
#define thash_set_uint(a, b, c) (thash_set((a), (b), ((const void *) (unsigned long) c)))

/** A safe thash_set in case indices are of unsigned int type */
#define thash_uint_set(a, b, c) (thash_set((a), ((const void *) (unsigned long) b), (c)))

/** Returns number of entries in a hash table.
 *
 * @param hash the hash table
 */
unsigned int thash_count(thash *hash);

/** Print thash contents
 * @note both key and value must be strings
 * @param lvl   level to dbg() */
void thash_dump(int lvl, thash *hash);

/** Clone a thash using given mmatic
 * @note hash table needs to be a pure "string -> string" one */
thash *thash_clone(thash *hash, void *mm);

/** Copy one thash to another making new copies of values
 * @param dst     destination thash
 * @param src     source thash
 * @return dst */
thash *thash_merge(thash *dst, thash *src);

/*
 * Hashing functions
 */

/** Generic string hashing function
 * @remark note that key will be dereferenced and treated like a string
 * @note by Daniel J. Bernstein */
unsigned int thash_str_hash(const void *vkey);

/** Simplest possible pointer "hashing" function
 * @note just casts the pointer */
unsigned int thash_ptr_hash(const void *key);

#endif

/*
 * vim: textwidth=100
 */
