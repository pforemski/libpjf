/*
 * thash - trivial hash table
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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _THASH_H_
#define _THASH_H_

#include <stdlib.h>

#include "mmatic.h"

/** Default size of new hash table. */
#define THASH_DEFAULT_SIZE 100

/** Maximal hash table usage before doubling it's size */
#define THASH_MAX_USAGE 0.60

/** Element of hash table */
typedef struct thash_el_t {
	/** Hash key used to access this field. */
	void *key;

	/** Element value. */
	void *val;

	/** Pointer at next element under this key.
	 * Used in case of key duplicates. */
	struct thash_el_t *next;
} thash_el;


/** A hash table */
typedef struct thash_t {
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
	mmatic *mm;

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
                    void (*free_func)(void *val), bool strings, mmatic *mm);

#define MMTHASH_CREATE_STR(ffn) (thash_create(NULL, NULL, (ffn), 1, mm))
#define MMTHASH_CREATE_PTR(ffn) (thash_create(NULL, NULL, (ffn), 0, mm))

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
 * @retval NULL entry does not exist
 */
inline void *thash_get(const thash *hash, const void *key);

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

/** Returns number of entries in a hash table.
 *
 * @param hash the hash table
 */
unsigned int thash_count(thash *hash);

/*
 * Hashing functions
 */

/** Generic string hashing function
 * @remark note that key will be dereferenced and treated like a string
 * @note by Daniel J. Bernstein
 */
inline unsigned int thash_str_hash(const void *key);

/** Simplest possible pointer "hashing" function
 * @note just casts the pointer
 */
inline unsigned int thash_ptr_hash(const void *key);

#endif

/*
 * vim: textwidth=100
 */
