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

#include <stdarg.h>

#include "lib.h"

#define tmmatic_alloc(mm, size) (mmatic_alloc(list, (size)))

tlist *tlist_create(void (*free_func)(void *val), void *mm)
{
	tlist *ret = mmatic_alloc(mm, sizeof(tlist));

	ret->head = ret->tail = ret->current = NULL;
	ret->free_func = free_func;
	ret->size = 0;

	return ret;
}

tlist *tlist_listify(void (*free_func)(void *val), void *mm, ...)
{
	va_list args;
	const void *arg;
	tlist *ret;

	ret = tlist_create(free_func, mm);
	if (!ret) return NULL;

	va_start(args, mm);
	while ((arg = va_arg(args, const void *))) tlist_push(ret, arg);
	va_end(args);

	return ret;
}

void tlist_flush(tlist *list)
{
	void *val;

	if (!list) return;

	while ((val = tlist_pop(list)))
		if (list->free_func)
			(list->free_func)(val);
}

void tlist_free(tlist *list)
{
	void *val;

	if (!list) return;

	if (list->free_func)
		while ((val = tlist_pop(list)))
			(list->free_func)(val);

	mmatic_free(list);
}

void tlist_reset(tlist *list) { if (list) list->current = list->head; }
void tlist_resetend(tlist *list) { if (list) list->current = list->tail; }
int tlist_size(tlist *list) { return list ? list->size : 0; }

void *tlist_iter_inc(tlist *list, int i)
{
	void *val;

	if (!list) return NULL;

	while (list->current && --i > 0) list->current = list->current->next;
	if (!list->current) return NULL;

	val = list->current->val;

	if (i >= 0) list->current = list->current->next;
	return val;
}

void *tlist_iter_dec(tlist *list, int i)
{
	void *val;

	if (!list) return NULL;

	while (list->current && --i > 0) list->current = list->current->prev;
	if (!list->current) return NULL;

	val = list->current->val;

	if (i >= 0) list->current = list->current->prev;
	return val;
}

void tlist_push(tlist *list, const void *val)
{
	tlist_el *el;

	if (!list) return;
	el = tmmatic_alloc(mm, sizeof(tlist_el));
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

void tlist_prepend(tlist *list, const void *val)
{
	tlist_el *el;

	if (!list) return;
	el = tmmatic_alloc(mm, sizeof(tlist_el));
	el->val = (void *) val;
	el->list = list;
	el->next = el->prev = NULL;

	if (!list->head) {
		list->head = list->tail = el;
	} else {
		el->next = list->head;
		list->head->prev = el;
		list->head = el;
	}

	list->size++;
}

void *tlist_pop(tlist *list)
{
	tlist_el *el;
	void *val;

	if (!list || !list->tail) return NULL;

	el = list->tail;
	val = el->val;

	if (el->prev) {
		el->prev->next = NULL;
		list->tail = el->prev;
	}
	else {
		list->head = list->tail = list->current = NULL;
	}

	mmatic_free(el);
	list->size--;

	return val;
}

void *tlist_shift(tlist *list)
{
	tlist_el *el;
	void *val;

	if (!list || !list->head) return NULL;

	el = list->head;
	val = el->val;

	if (list->current == el)
		list->current = el->next;

	if (el->next) {
		el->next->prev = NULL;
		list->head = el->next;
	}
	else {
		list->head = list->tail = list->current = NULL;
	}

	mmatic_free(el);
	list->size--;

	return val;
}

void *tlist_remove(tlist *list)
{
	tlist_el *el;
	void *val;

	if (!list) return NULL;

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

	mmatic_free(el);
	list->size--;

	return val;
}

#define INSERT_EL(el_next, el_prev, el_check) \
	tlist_el *el; \
	if (!list->current) { \
		tlist_push(list, val); \
		return; \
	} \
	el = tmmatic_alloc(mm, sizeof(tlist_el)); \
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
	if (!list) return;
	if (!list->current) tlist_reset(list);
	INSERT_EL(list->current, list->current->prev, list->head);
}

void tlist_insertafter(tlist *list, const void *val)
{
	/* XXX: assuming dereference of list->current->next AFTER checking if list->current != NULL */
	if (!list) return;
	INSERT_EL(list->current->next, list->current, list->tail);
}

char *tlist_stringify(tlist *list, const char *sep, void *mm)
{
	int l = 0, sl = strlen(sep);
	char *s, *ret, *p;

	if (!list) return NULL;

	tlist_reset(list);
	while ((s = tlist_iter(list)))
		l += strlen(s) + sl;

	if (l <= 0) return mmatic_strdup(mm, "");
	p = ret = mmatic_alloc(mm, l);

	tlist_reset(list);
	while ((s = tlist_iter(list))) {
		l = strlen(s);
		memcpy(p,   s,   l);
		memcpy(p+l, sep, sl);
		p += l + sl;
	}

	p -= sl;
	*p = '\0'; /* we dont want the last separator */

	return ret;
}

/*
 * vim: textwidth=100
 */
