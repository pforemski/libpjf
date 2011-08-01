/*
 * tlist - trivial list
 *
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Authors: Pawel Foremski <pawel@foremski.pl>
 *          Łukasz Zemczak <sil2100@asn.pl>
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

#ifndef _TLIST_H_
#define _TLIST_H_

#include "mmatic.h"

typedef struct tlist_el tlist_el;

/** A list */
typedef struct tlist {
	/** Head */
	tlist_el *head;

	/** Tail */
	tlist_el *tail;

	/** Iterator */
	tlist_el *current;

	/** Number of elements stored in the list */
	int size;

	/** Function used to free memory occupied by an element. */
	void (*free_func)(void *val);
} tlist;

/** Element of list */
struct tlist_el {
	/** Element value. */
	void *val;

	/** Next element */
	tlist_el *next;

	/** Previous element */
	tlist_el *prev;

	/** List root */
	tlist *list;
};

/** Creates a list
 * @param  free_func function to use to free an element; if NULL, then memory won't be freed
 * @note   always succeeds
 */
tlist *tlist_create(void (*free_func)(), void *mm);

/** Wrapper around tlist_create() which tlist_push()es given arguments
 * @param  free_func function to use to free an element; if NULL, then memory won't be freed
 * @param  ...       arguments to tlist_push(); REMEMBER to end the list with \0
 * @remark the last argument should be \0
 */
tlist *tlist_listify(void (*free_func)(), void *mm, ...);

/** Flushes a list
 * @param list the list to flush
 */
void tlist_flush(tlist *list);

/** Frees a list
 * @param list the list
 * @note not only all elements, but also the list itself (see also tlist_flush)
 */
void tlist_free(tlist *list);

/** Resets internal iterator to list beginning
 * @param list the list
 */
void tlist_reset(tlist *list);

/** Resets internal iterator to list end
 * @param list the list
 */
void tlist_resetend(tlist *list);

/** Iterate through list entries in forward direction
 * @param  list  the list
 * @param  i     step
 * @retval NULL  end of the list reached
 */
void *tlist_iter_inc(tlist *list, int i);

/** Iterate through list entries in backward direction
 * @param  list  the list
 * @param  i     step
 * @retval NULL  beginning of the list reached
 */
void *tlist_iter_dec(tlist *list, int i);

#define tlist_peek(list)      (tlist_iter_inc((list), 0))
#define tlist_iter(list)      (tlist_iter_inc((list), 1))
#define tlist_iterback(list)  (tlist_iter_dec((list), 1))

#define tlist_iter_loop(list, v) tlist_reset(list); while (((v) = tlist_iter(list)))

/** Pushes a value at the end of a list */
void tlist_push(tlist *list, const void *val);

/** Pushes a value at the beginning of a list */
void tlist_prepend(tlist *list, const void *val);

/** Pops a value off the end */
void *tlist_pop(tlist *list);

/** Shift whole list left - ie. removes element at the beginning */
void *tlist_shift(tlist *list);

/** Insert value before the element pointed by the iterator
 *  If current iterator is NULL, push the value at the beginning
 *
 * @param list the list
 * @param val  value of the element
 */
void tlist_insertbefore(tlist *list, const void *val);

/** Insert value after the element pointed by the iterator
 *  If current iterator is NULL, push the value at the end
 *
 * @param list the list
 * @param val  value of the element
 */
void tlist_insertafter(tlist *list, const void *val);

/** Remove value from the position pointed by the iterator-1
 *
 * Note that removing iterator-1 is designed to be used with tlist_iter()
 *
 * @param list the list
 * @return value of removed item
 * @retval NULL if iterator not set properly
 */
void *tlist_remove(tlist *list);

/** Returns the number of elements in the list */
int tlist_size(tlist *list);
#define tlist_count tlist_size

/** Stringify a tlist using given separator
 * @param  list    list to stringify
 * @param  sep     separator (cant be null!)
 * @return char *  (always, even just an mm-ed "") */
char *tlist_stringify(tlist *list, const char *sep, void *mm);

#endif

/*
 * vim: textwidth=100
 */
