/*
 * thash - trivial hash table
 *
 * This file is part of libpjf
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

#include <stdlib.h>
#include <string.h>

#include "lib.h"

static int _thash_strcmp_wrapper(const void *key1, const void *key2)
{
	return strcmp((const char *) key1, (const char *) key2);
}

/* generic comparisiong function */
static int _thash_compare(const void *key1, const void *key2)
{
	return (((unsigned long) key1) != ((unsigned long) key2));
}

#define _thash_alloc(hash, size) (((hash)->mm) ? mmatic_alloc((hash)->mm, (size)) : pjf_malloc(size));
#define _thash_free(hash, ptr)   (((hash)->mm) ? mmfreeptr(ptr) : free(ptr));

thash *thash_create(unsigned int (*hash_func)(const void *key),
                    int (*cmp_func)(const void *key1, const void *key2),
                    void (*free_func)(void *val), bool strings, void *mm)
{
	thash *hash;

	if (mm)
		hash = mmalloc(sizeof(thash));
	else
		hash = pjf_malloc(sizeof(thash));

	hash->size = THASH_DEFAULT_SIZE;
	hash->used = 0;
	hash->mm = mm;

	hash->tbl = _thash_alloc(hash, hash->size * sizeof(thash_el *));
	memset(hash->tbl, 0, hash->size * sizeof(thash_el *));

	if (hash_func)
		hash->hash_func = hash_func;
	else
		hash->hash_func = strings ? thash_str_hash : thash_ptr_hash;

	if (cmp_func)
		hash->cmp_func = cmp_func;
	else
		hash->cmp_func = strings ? _thash_strcmp_wrapper : _thash_compare;

	hash->free_func = free_func;
	hash->strings_mode = strings ? 1 : 0;

	thash_reset(hash);
	return hash;
}

void thash_flush(thash *hash)
{
	unsigned int i;
	thash_el *el, *el2;

	if (!hash) return;

	for (i = 0; i < hash->size; i++) {
		el = hash->tbl[i];
		hash->tbl[i] = 0;
		while (el) {
			if (hash->strings_mode)
				_thash_free(hash, el->key);
			if (hash->free_func)
				(hash->free_func)(el->val);
			el2 = el->next;
			_thash_free(hash, el);
			el = el2;
		}
	}

	hash->used = 0;
	hash->counter_x = 0;
	hash->counter_y = 0;
}

void thash_free(thash *hash)
{
	if (!hash) return;
	thash_flush(hash);
	_thash_free(hash, hash->tbl);
	_thash_free(hash, hash);
}

void *_thash_iter(thash *hash, void **key)
{
	unsigned int i;
	thash_el *el;

	if (!hash) return NULL;

	/* we're at the end */
	if (hash->counter_x >= hash->size) return NULL;

	/* find next defined element in table */
	do {
		el = hash->tbl[hash->counter_x];
	} while (!el && ++hash->counter_x < hash->size);

	/* if not found - notify caller we're at the end */
	if (!el) return NULL;

	/* find next element in list */
	for (i = 0; i < hash->counter_y && el->next; i++)
		el = el->next;

	/* decide where to search in next call */
	if (el->next) {
		hash->counter_y++;
	}
	else {
		hash->counter_y = 0;
		hash->counter_x++;
	}

	if (key != NULL) *key = el->key;
	return el->val;
}

void thash_reset(thash *hash) { if (hash) hash->counter_x = hash->counter_y = 0; }

/** Resizes a hash table.
 * It doesn't use realloc() because table indices will change as they depend on hash->size.
 */
static void thash_resize(thash *hash)
{
	unsigned int old_size;
	thash_el **old_tbl, *el, *el2;
	int i;

	if (!hash) return;

	old_size = hash->size;
	old_tbl = hash->tbl;

	/* XXX: should guarantee we won't be called recursively while copying
	 *      old data with thash_set */
	hash->size *= 2;

	hash->tbl = _thash_alloc(hash, hash->size * sizeof(thash_el *));

	/* zero new memory */
	memset(hash->tbl, 0, hash->size * sizeof(thash_el *));
	hash->used = 0;

	/* XXX: copy old data as indices might have changed (they depend
	 * on hash->size) */
	for (i = 0; i < old_size; i++) {
		el = old_tbl[i];
		while (el) {
			thash_set(hash, el->key, el->val);
			el2 = el;
			el = el->next;
			_thash_free(hash, el2);
		}
	}

	/* free old data */
	_thash_free(hash, old_tbl);
}

void thash_set(thash *hash, const void *key, const void *val)
{
	int i;
	unsigned int index;
	thash_el *root_el, *el = NULL, *parent_el = NULL, *itel;

	if (!hash) return;

	/* resize table if usage ratio > THASH_MAX_USAGE */
	if ((double) hash->used / hash->size > THASH_MAX_USAGE)
		thash_resize(hash);

	index = (hash->hash_func)(key) % hash->size;
	root_el = hash->tbl[index];

	/* check if entry already exists */
	el = root_el;
	while (el && (hash->cmp_func)(el->key, key)) {
		dbg(12, "thash_set(): collision for key %p (index %d) with %p\n", key, index, el->key);
		parent_el = el;
		el = el->next;
	}

	/* create an entry */
	if (!el) {
		if (!val) return;        /* deletion "done" */
		dbg(12, "thash_set(): adding %p\n", key);

		el = _thash_alloc(hash, sizeof(thash_el));

		if (hash->strings_mode) {
			el->key = _thash_alloc(hash, strlen(key) + 1);
			strcpy(el->key, key);
		}
		else {
			el->key = (void *) key;
		}

		el->val = (void *) val;
		el->next = NULL;

		if (!root_el)
			hash->tbl[index] = el;
		else
			parent_el->next = el;

		/* increase usage counter */
		hash->used++;
	}
	else if (val) { /* update */
		/* XXX: old value not freed */
		el->val = (void *) val;
	}
	else { /* val = null, delete */
		if (hash->strings_mode)
			_thash_free(hash, el->key);

		if (hash->free_func)
			(hash->free_func)(el->val);

		/* handle iterator */
		if (index == hash->counter_x && hash->counter_y > 0) {
			/* find last element returned by thash_iter() */
			itel = root_el;
			for (i = 0; i < hash->counter_y - 1 && itel->next; i++)
				itel = itel->next;

			/* if this is the element being deleted, decrease the y counter so that the element just
			 * after it is not skipped in next call to thash_iter() */
			if (el == itel)
				hash->counter_y--;
		}

		if (parent_el) {
			parent_el->next = el->next;
		} else {
			hash->tbl[index] = el->next;
		}

		_thash_free(hash, el);
		hash->used--;
	}

	return;
}

void *thash_get(const thash *hash, const void *key)
{
	thash_el *el;
	void *val = NULL;
	int index;

	if (!hash) return NULL;

	index = (hash->hash_func)(key) % hash->size;

	el = hash->tbl[index];
	while (el) {
		if ((hash->cmp_func)(el->key, key) == 0) {
			val = el->val;
			break;
		}
		el = el->next;
	}

	dbg(12, "thash_get(%p, %p) = %p\n", hash, key, val);
	return val;
}

unsigned int thash_count(thash *hash)
{
	return hash ? hash->used : 0;
}

void thash_dump(int lvl, thash *hash)
{
	char *v, *k;

	if (!hash) return;

	thash_reset(hash);
	while ((v = thash_iter(hash, &k)))
		dbg(lvl, "%s = %s\n", k, v);
}

thash *thash_clone(thash *hash, void *mm)
{
	thash *ret;
	const char *k, *v;

	if (!hash) return NULL;

	ret = thash_create(
		hash->hash_func, hash->cmp_func, hash->free_func,
		hash->strings_mode, mm);

	thash_iter_loop(hash, k, v)
		thash_set(ret, k, mmstrdup(v));

	return ret;
}

thash *thash_merge(thash *dst, thash *src)
{
	const char *k, *v;

	if (!dst) return NULL;
	if (!src) return dst;

	thash_iter_loop(src, k, v) {
		thash_set(dst, k, mmatic_copyto(v, dst));
	}

	return dst;
}

unsigned int thash_str_hash(const void *vkey)
{
	unsigned int hash = 5381;
	unsigned int i;
	const char *key = (char *) vkey;

	for (i = 0; key[i]; i++)
		hash = ((hash << 5) + hash) + key[i]; /* hash * 33 + char */

	return hash;
}

unsigned int thash_ptr_hash(const void *key)
{
	return ((unsigned long) key);
}
