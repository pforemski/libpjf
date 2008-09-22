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

#include <stdarg.h>

#include "misc.h"
#include "tlist.h"
#include "mmatic.h"

#define tmmalloc(size) (mmatic_alloc((size), list->mm))

tlist *tlist_create(void (*free_func)(void *val), mmatic *mm)
{
	tlist *ret = mmalloc(sizeof(tlist));

	ret->head = ret->tail = ret->current = NULL;
	ret->free_func = free_func;
	ret->mm = mm;
	ret->size = 0;

	return ret;
}

tlist *tlist_listify(void (*free_func)(void *val), mmatic *mm, ...)
{
	va_list args;
	const void *arg;
	tlist *ret;

	ret = mmtlist_create(free_func);
	if (!ret) return NULL;

	va_start(args, mm);
	while ((arg = va_arg(args, const void *))) tlist_push(ret, arg);
	va_end(args);

	return ret;
}

void tlist_flush(tlist *list)
{
	void *val;

	while ((val = tlist_pop(list)))
		if (list->free_func)
			(list->free_func)(val);
}

void tlist_free(tlist *list)
{
	void *val;

	if (list->free_func)
		while ((val = tlist_pop(list)))
			(list->free_func)(val);

	mmfreeptr(list);
}

void tlist_reset(tlist *list) { list->current = list->head; }
void tlist_resetend(tlist *list) { list->current = list->tail; }
int tlist_size(tlist *list) { return list->size; }

void *tlist_iter(tlist *list)
{
	void *val;

	if (!list->current)
		return NULL;

	val = list->current->val;
	list->current = list->current->next;

	return val;
}

void *tlist_iterback(tlist *list)
{
	void *val;

	if (!list->current)
		return NULL;

	val = list->current->val;
	list->current = list->current->prev;

	return val;
}

void tlist_push(tlist *list, const void *val)
{
	tlist_el *el = tmmalloc(sizeof(tlist_el));

	el->val = (void *) val;
	el->list = list;
	el->next = el->prev = NULL;

	if (!list->tail) {
		list->head = list->tail = el;
	}
	else {
		el->prev = list->tail;
		list->tail->next = el;
		list->tail = el;
	}

	list->size++;
}

void *tlist_pop(tlist *list)
{
	tlist_el *el;
	void *val;

	if (!list->tail) return NULL;

	el = list->tail;
	val = el->val;

	if (list->tail->prev) {
		list->tail->prev->next = NULL;
		list->tail = list->tail->prev;
	}
	else {
		list->head = list->tail = list->current = NULL;
	}

	mmfreeptr(el);
	list->size--;

	return val;
}

void *tlist_remove(tlist *list)
{
	tlist_el *el;
	void *val;

	if (list->current)
		el = (list->current->prev) ? list->current->prev : list->current;
	else
		el = list->tail; /* good point, Watson ;-) */

	if (!el) return NULL;
	val = el->val;

	if (el->prev)
		el->prev->next = el->next;
	if (el->next)
		el->next->prev = el->prev;

	if (el == list->head)
		list->head = el->next;
	if (el == list->tail)
		list->tail = el->prev;
	if (el == list->current)
		list->current = el->next;

	mmfreeptr(el);
	list->size--;

	return val;
}

#define INSERT_EL(el_next, el_prev, el_check) \
	tlist_el *el; \
	if (!list->current) { \
		tlist_push(list, val); \
		return; \
	} \
	el = tmmalloc(sizeof(tlist_el)); \
	el->val = (void *) val; \
	el->list = list; \
	el->next = el_next; \
	el->prev = el_prev; \
	if (el->next) el->next->prev = el; \
	if (el->prev) el->prev->next = el; \
	if (list->current == el_check) el_check = el; \
	list->size++;

void tlist_insertbefore(tlist *list, const void *val)
{
	INSERT_EL(list->current, list->current->prev, list->head);
}

/*
 * vim: textwidth=100
 */
