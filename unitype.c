/*
 * This file is part of libpjf
 * Copyright (C) 2009-2010 ASN Sp. z o.o.
 * Author: Pawel Foremski <pf:remski@asn.pl>
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

#define mm var

bool   ut_bool(ut *var)
{
	if (!var) return false;

	switch (var->type) {
		case T_BOOL:   return var->d.as_bool;
		case T_INT:    return (bool) var->d.as_int;
		case T_UINT:   return (bool) var->d.as_uint;
		case T_DOUBLE: return (bool) var->d.as_double;
		case T_STRING: return (bool) ut_int(var);
		case T_LIST:   return (tlist_count(var->d.as_tlist) > 0);
		case T_HASH:   return (thash_count(var->d.as_thash) > 0);
		default:       return false;
	}
}

int    ut_int(ut *var)
{
	if (!var) return 0;

	switch (var->type) {
		case T_INT:    return var->d.as_int;
		case T_UINT:   return (int) var->d.as_uint;
		case T_DOUBLE: return (int) var->d.as_double;
		case T_STRING: return atoi(xstr_string(var->d.as_xstr));
		default: return 0;
	}
}

uint32_t ut_uint(ut *var)
{
	if (!var) return 0;

	switch (var->type) {
		case T_UINT:   return var->d.as_uint;
		case T_INT:    return (uint32_t) var->d.as_int;
		case T_DOUBLE: return (uint32_t) var->d.as_double;
		case T_STRING: return strtoul(xstr_string(var->d.as_xstr), NULL, 10);
		default: return 0;
	}
}

double ut_double(ut *var)
{
	if (!var) return 0.0;

	switch (var->type) {
		case T_DOUBLE: return var->d.as_double;
		case T_INT:    return (double) var->d.as_int;
		case T_UINT:   return (double) var->d.as_uint;
		case T_STRING: return strtod(xstr_string(var->d.as_xstr), NULL);
		default: return 0.0;
	}
}

xstr  *ut_xstr(ut *var)
{
	if (!var) return NULL;

	char buf[BUFSIZ], *key;
	xstr *xs;
	ut *el;

	switch (var->type) {
		case T_STRING:
			return var->d.as_xstr;
		case T_INT:
			snprintf(buf, sizeof buf, "%d", var->d.as_int);
			return MMXSTR_CREATE(buf);
		case T_UINT:
			snprintf(buf, sizeof buf, "%u", var->d.as_uint);
			return MMXSTR_CREATE(buf);
		case T_DOUBLE:
			snprintf(buf, sizeof buf, "%g", var->d.as_double);
			return MMXSTR_CREATE(buf);
		case T_LIST:
			xs = MMXSTR_CREATE("");
			tlist_iter_loop(var->d.as_tlist, el) {
				xstr_append(xs, ut_char(el));
				xstr_append_char(xs, ' ');
			}
			return xs;
		case T_HASH:
			xs = MMXSTR_CREATE("");
			thash_iter_loop(var->d.as_thash, key, el) {
				xstr_append(xs, key);
				xstr_append(xs, ": ");
				xstr_append(xs, ut_char(el));
				xstr_append_char(xs, ' ');
			}
			return xs;
		case T_BOOL:
			if (var->d.as_bool)
				return MMXSTR_CREATE("true");
			else
				return MMXSTR_CREATE("false");
		case T_PTR:
			snprintf(buf, sizeof buf, "%p", var->d.as_ptr);
			return MMXSTR_CREATE(buf);
		case T_ERR:
			return MMXSTR_CREATE(ut_err(var));
		default:
			return MMXSTR_CREATE("");
	}
}

const char *ut_char(ut *var)
{
	if (!var) return NULL;

	return xstr_string(ut_xstr(var));
}

tlist *ut_tlist(ut *var)
{
	if (!var) return NULL;

	tlist *list;
	char *key;
	ut *el;

	switch (var->type) {
		case T_LIST:
			return var->d.as_tlist;
		case T_HASH:
			list = tlist_create(NULL, mm);
			thash_iter_loop(var->d.as_thash, key, el)
				tlist_push(list, el);
			return list;
		default:
			return tlist_create(NULL, mm);
	}
}

thash *ut_thash(ut *var)
{
	if (!var) return NULL;

	switch (var->type) {
		case T_HASH:
			return var->d.as_thash;
		default:
			return thash_create_strkey(NULL, mm);
	}
}

void  *ut_ptr(ut *var)
{
	if (!var) return NULL;

	switch (var->type) {
		case T_PTR:
			return var->d.as_ptr;
		case T_HASH:
			return var->d.as_thash;
		case T_LIST:
			return var->d.as_tlist;
		case T_STRING:
			return var->d.as_xstr;
		default:
			return &(var->d);
	}
}

const char *ut_err(ut *var)
{
	if (var && var->type == T_ERR) {
		if (var->d.as_err->data)
			return mmprintf("#%d: %s (%s)", var->d.as_err->code, var->d.as_err->msg, var->d.as_err->data);
		else
			return mmprintf("#%d: %s", var->d.as_err->code, var->d.as_err->msg);
	}
	else {
		return "";
	}
}

int ut_errcode(ut *var)
{
	if (var && var->type == T_ERR)
		return var->d.as_err->code;
	else
		return 0;
}

/****************************************************************/

ut *ut_new_bool(bool val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_BOOL;
	ret->d.as_bool= val;

	return ret;
}

ut *ut_new_int(int val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_INT;
	ret->d.as_int = val;

	return ret;
}

ut *ut_new_uint(uint32_t val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_UINT;
	ret->d.as_uint = val;

	return ret;
}

ut *ut_new_double(double val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_DOUBLE;
	ret->d.as_double = val;

	return ret;
}

ut *ut_new_char(const char *val, void *mm)
{
	return ut_new_xstr(xstr_create(val ? val : "", mm), mm);
}

ut *ut_new_xstr(xstr *val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_STRING;
	ret->d.as_xstr = val ? val : MMXSTR_CREATE("");

	return ret;
}

ut *ut_new_uttlist(tlist *val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_LIST;
	ret->d.as_tlist = val ? val : tlist_create(NULL, mm); /* TODO: make a freeing system */

	return ret;
}

ut *ut_new_tlist(tlist *val, void *mm)
{
	char *v;
	ut *ret = ut_new_uttlist(NULL, mm);

	if (val) { tlist_iter_loop(val, v) utl_add_char(ret, v); }
	return ret;
}

ut *ut_new_utthash(thash *val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_HASH;
	ret->d.as_thash = val ? val : thash_create_strkey(NULL, mm); /* TODO: make a freeing system */

	return ret;
}

ut *ut_new_thash(thash *val, void *mm)
{
	char *k, *v;
	ut *ret = ut_new_utthash(NULL, mm);

	if (val) { thash_iter_loop(val, k, v) uth_set_char(ret, k, v); }
	return ret;
}

ut *ut_new_ptr(void *val, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_PTR;
	ret->d.as_ptr = val;

	return ret;
}

ut *ut_new_null(void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_NULL;

	return ret;
}

ut *ut_new_err(int code, const char *msg, const char *data, void *mm)
{
	ut *ret = mmatic_alloc(mm, sizeof(struct ut));

	ret->type = T_ERR;
	ret->d.as_err = mmatic_alloc(mm, sizeof(struct ut_err));

	ret->d.as_err->code = code;
	ret->d.as_err->msg  = msg ? msg : "";
	ret->d.as_err->data = data;

	return ret;
}

/****************************************************************/

ut *uth_get(ut *var, const char *key)
{
	return (ut_is_thash(var) ? thash_get(var->d.as_thash, key) : NULL);
}

ut *uth_set(ut *var, const char *key, ut *val)
{
	if (ut_is_thash(var))
		thash_set(var->d.as_thash, key, val);
	return val;
}

ut *uth_set_null(ut *var, const char *key)
{
	return uth_set(var, key, ut_new_null(var));
}

ut *uth_set_bool(ut *var, const char *key, bool val)
{
	return uth_set(var, key, ut_new_bool(val, var));
}

ut *uth_set_int(ut *var, const char *key, int val)
{
	return uth_set(var, key, ut_new_int(val, var));
}

ut *uth_set_uint(ut *var, const char *key, uint32_t val)
{
	return uth_set(var, key, ut_new_uint(val, var));
}

ut *uth_set_double(ut *var, const char *key, double val)
{
	return uth_set(var, key, ut_new_double(val, var));
}

ut *uth_set_char(ut *var, const char *key, const char *val)
{
	return uth_set(var, key, ut_new_char(val, var));
}

ut *uth_set_xstr(ut *var, const char *key, xstr *val)
{
	return uth_set(var, key, ut_new_xstr(val, var));
}

ut *uth_set_tlist(ut *var, const char *key, tlist *val)
{
	return uth_set(var, key, ut_new_tlist(val, var));
}

ut *uth_set_thash(ut *var, const char *key, thash *val)
{
	return uth_set(var, key, ut_new_thash(val, var));
}

ut *uth_set_ptr(ut *var, const char *key, void *ptr)
{
	return uth_set(var, key, ut_new_ptr(ptr, var));
}

ut *uth_merge(ut *dst, ut *src)
{
	if (ut_is_thash(src) && ut_is_thash(dst))
		thash_merge(dst->d.as_thash, src->d.as_thash);

	return dst;
}

ut *uth_path_get_(ut *node, const char *key, ...)
{
	va_list keys;

	va_start(keys, key);
	while (key && *key) {
		if (!ut_is_thash(node)) {
			key = va_arg(keys, const char *);

			/* if there is still path to traverse, current node (of different type than a thash) was not the last path
			 * element and thus we cannot go further - fail */
			if (key && *key)
				node = NULL;

			break;
		}

		node = uth_get(node, key);
		key = va_arg(keys, const char *);
	}

	va_end(keys);
	return node;
}

ut *uth_path_create_(ut *parent, const char *key, ...)
{
	va_list keys;
	ut *child;

	pjf_assert(ut_is_thash(parent));

	va_start(keys, key);
	while (key && *key) {
		child = uth_get(parent, key);

		if (!ut_is_thash(child))
			child = uth_set_thash(parent, key, NULL);

		parent = child;
		key = va_arg(keys, const char *);
	}

	va_end(keys);
	return child;
}

/****************************************************************/

ut *utl_add(ut *var, ut *val)
{
	if (ut_is_tlist(var))
		tlist_push(var->d.as_tlist, val);
	return val;
}

ut *utl_add_null(ut *var)
{
	return utl_add(var, ut_new_null(var));
}

ut *utl_add_bool(ut *var, bool val)
{
	return utl_add(var, ut_new_bool(val, var));
}

ut *utl_add_int(ut *var, int val)
{
	return utl_add(var, ut_new_int(val, var));
}

ut *utl_add_uint(ut *var, uint32_t val)
{
	return utl_add(var, ut_new_uint(val, var));
}

ut *utl_add_double(ut *var, double val)
{
	return utl_add(var, ut_new_double(val, var));
}

ut *utl_add_char(ut *var, const char *val)
{
	return utl_add(var, ut_new_char(val, var));
}

ut *utl_add_xstr(ut *var, xstr *val)
{
	return utl_add(var, ut_new_xstr(val, var));
}

ut *utl_add_tlist(ut *var, tlist *val)
{
	return utl_add(var, ut_new_tlist(val, var));
}

ut *utl_add_thash(ut *var, thash *val)
{
	return utl_add(var, ut_new_thash(val, var));
}

ut *utl_add_ptr(ut *var, void *ptr)
{
	return utl_add(var, ut_new_ptr(ptr, var));
}
