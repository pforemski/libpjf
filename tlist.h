/*
 * tlist - trivial list
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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TLIST_H_
#define _TLIST_H_

#include <libasn/mmatic.h>

typedef struct _tlist_el tlist_el;

/** A list */
typedef struct _tlist {
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

	/** mmatic memory */
	mmatic *mm;
} tlist;

/** Element of list */
struct _tlist_el {
	/** Element value. */
	void *val;

	/** Next element */
	struct _tlist_el *next;

	/** Previous element */
	struct _tlist_el *prev;

	/** List root */
	tlist *list;
};

/** Creates a list
 *
 * @param  free_func function to use to free an element; if NULL, then memory won't be freed
 * @note   always succeeds
 */
tlist *tlist_create(void (*free_func)(void *val), mmatic *mm);

/** Wrapper around tlist_create() which tlist_push()es given arguments
 *
 * @param  free_func function to use to free an element; if NULL, then memory won't be freed
 * @param  ...       arguments to tlist_push(); REMEMBER to end the list with \0
 * @remark the last argument should be \0
 */
tlist *tlist_listify(void (*free_func)(void *val), mmatic *mm, ...);

/** Flushes a list
 *
 * @param list the list to flush
 */
void tlist_flush(tlist *list);

/** Frees a list
 *
 * @param list the list
 */
void tlist_free(tlist *list);

/** Resets internal iterator to list beginning
 *
 * @param list the list
 */
void tlist_reset(tlist *list);

/** Resets internal iterator to list end
 *
 * @param list the list
 */
void tlist_resetend(tlist *list);

/** Iterate through list entries
 *
 * @param list the list
 * @retval NULL  end of list reached
 */
void *tlist_iter(tlist *list);

/** Iterates back
 *
 * @param list  the list
 * @retval NULL beginning of list reached
 */
void *tlist_iterback(tlist *list);

/** Pushes a value at the end */
void tlist_push(tlist *list, const void *val);

/** Pops a value off end */
void *tlist_pop(tlist *list);

/** Insert value before the element pointed by the iterator
 *  If current iterator is NULL, push the value at the end
 *
 * @param list the list
 * @param val  value of the element
 */
void tlist_insertbefore(tlist *list, const void *val);

/** Insert value after the element
 *  If current iterator is NULL, push the value at the end
 *
 * @param list the list
 * @param val  value of the element
 */
void tlist_insertafter(tlist *list, const void *val);

/** Remove value from the position pointed at by the iterator - 1
 *
 * @param list the list
 * @retval NULL if iterator not set properly
 */
void *tlist_remove(tlist *list);

/** Returns the number of elements in the list */
int tlist_size(tlist *list);
#define tlist_count tlist_size

/***/
#define MMTLIST_CREATE(fn) (tlist_create((fn), mm))

#endif

/*
 * vim: textwidth=100
 */
