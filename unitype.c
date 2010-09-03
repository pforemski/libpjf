/*
 * This file is part of libasn
 * Copyright (C) 2009 ASN Sp. z o.o.
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

#include "lib.h"

bool   ut_bool(ut *var)
{
	if (!var) return false;

	switch (var->type) {
		case T_BOOL:   return var->d.as_bool;
		case T_INT:    return (bool) var->d.as_int;
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
		case T_DOUBLE: return (int) var->d.as_double;
		case T_STRING: return atoi(xstr_string(var->d.as_xstr));
		default: return 0;
	}
}

double ut_double(ut *var)
{
	if (!var) return 0.0;

	switch (var->type) {
		case T_DOUBLE: return var->d.as_double;
		case T_INT:    return (double) var->d.as_int;
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
	mmatic *mm = var->mm;

	switch (var->type) {
		case T_STRING:
			return var->d.as_xstr;
		case T_INT:
			snprintf(buf, sizeof(buf), "%d", var->d.as_int);
			return MMXSTR_CREATE(buf);
		case T_DOUBLE:
			snprintf(buf, sizeof(buf), "%g", var->d.as_double);
			return MMXSTR_CREATE(buf);
		case T_LIST:
			xs = MMXSTR_CREATE("");
			TLIST_ITER_LOOP(var->d.as_tlist, el) {
				xstr_append(xs, ut_char(el));
				xstr_append_char(xs, ' ');
			}
			return xs;
		case T_HASH:
			xs = MMXSTR_CREATE("");
			THASH_ITER_LOOP(var->d.as_thash, key, el) {
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
			snprintf(buf, sizeof(buf), "%p", var->d.as_ptr);
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
	mmatic *mm = var->mm;

	switch (var->type) {
		case T_LIST:
			return var->d.as_tlist;
		case T_HASH:
			list = MMTLIST_CREATE(NULL);
			THASH_ITER_LOOP(var->d.as_thash, key, el)
				tlist_push(list, el);
			return list;
		default:
			return MMTLIST_CREATE(NULL);
	}
}

thash *ut_thash(ut *var)
{
	if (!var) return NULL;

	mmatic *mm = var->mm;

	switch (var->type) {
		case T_HASH:
			return var->d.as_thash;
		default:
			return MMTHASH_CREATE_STR(NULL);
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
			return mmatic_printf(var->mm, "#%d: %s (%s)",
				var->d.as_err->code, var->d.as_err->msg, var->d.as_err->data);
		else
			return mmatic_printf(var->mm, "#%d: %s",
				var->d.as_err->code, var->d.as_err->msg);
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

ut *ut_new_bool(bool val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_BOOL;
	ret->d.as_bool= val;

	return ret;
}

ut *ut_new_int(int val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_INT;
	ret->d.as_int = val;

	return ret;
}

ut *ut_new_double(double val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_DOUBLE;
	ret->d.as_double = val;

	return ret;
}

ut *ut_new_char(const char *val, mmatic *mm)
{
	return ut_new_xstr(xstr_create(val ? val : "", mm), mm);
}

ut *ut_new_xstr(xstr *val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_STRING;
	ret->d.as_xstr = val ? val : MMXSTR_CREATE("");

	return ret;
}

ut *ut_new_uttlist(tlist *val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_LIST;
	ret->d.as_tlist = val ? val : MMTLIST_CREATE(NULL); /* TODO: make a freeing system */

	return ret;
}

ut *ut_new_tlist(tlist *val, mmatic *mm)
{
	char *v;
	ut *ret = ut_new_uttlist(NULL, mm);

	if (val) { TLIST_ITER_LOOP(val, v) utl_add_char(ret, v); }
	return ret;
}

ut *ut_new_utthash(thash *val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_HASH;
	ret->d.as_thash = val ? val : MMTHASH_CREATE_STR(NULL); /* TODO: make a freeing system */

	return ret;
}

ut *ut_new_thash(thash *val, mmatic *mm)
{
	char *k, *v;
	ut *ret = ut_new_utthash(NULL, mm);

	if (val) { THASH_ITER_LOOP(val, k, v) uth_set_char(ret, k, v); }
	return ret;
}

ut *ut_new_ptr(void *val, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_PTR;
	ret->d.as_ptr = val;

	return ret;
}

ut *ut_new_null(mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_NULL;

	return ret;
}

ut *ut_new_err(int code, const char *msg, const char *data, mmatic *mm)
{
	ut *ret = mmatic_alloc(sizeof(struct ut), mm);

	ret->mm = mm;
	ret->type = T_ERR;
	ret->d.as_err = mmatic_alloc(sizeof(struct ut_err), mm);

	ret->d.as_err->code = code;
	ret->d.as_err->msg  = msg ? msg : "";
	ret->d.as_err->data = data;

	return ret;
}

/****************************************************************/

ut *uth_get(ut *var, const char *key)
{
	return thash_get(var->d.as_thash, key);
}

ut *uth_set(ut *var, const char *key, ut *val)
{
	thash_set(var->d.as_thash, key, val);
	return val;
}

ut *uth_set_null(ut *var, const char *key)
{
	return uth_set(var, key, ut_new_null(var->mm));
}

ut *uth_set_bool(ut *var, const char *key, bool val)
{
	return uth_set(var, key, ut_new_bool(val, var->mm));
}

ut *uth_set_int(ut *var, const char *key, int val)
{
	return uth_set(var, key, ut_new_int(val, var->mm));
}

ut *uth_set_double(ut *var, const char *key, double val)
{
	return uth_set(var, key, ut_new_double(val, var->mm));
}

ut *uth_set_char(ut *var, const char *key, const char *val)
{
	return uth_set(var, key, ut_new_char(val, var->mm));
}

ut *uth_set_xstr(ut *var, const char *key, xstr *val)
{
	return uth_set(var, key, ut_new_xstr(val, var->mm));
}

ut *uth_set_tlist(ut *var, const char *key, tlist *val)
{
	return uth_set(var, key, ut_new_tlist(val, var->mm));
}

ut *uth_set_thash(ut *var, const char *key, thash *val)
{
	return uth_set(var, key, ut_new_thash(val, var->mm));
}

ut *uth_set_ptr(ut *var, const char *key, void *ptr)
{
	return uth_set(var, key, ut_new_ptr(ptr, var->mm));
}

/****************************************************************/

ut *utl_add(ut *var, ut *val)
{
	tlist_push(var->d.as_tlist, val);
	return val;
}

ut *utl_add_null(ut *var)
{
	return utl_add(var, ut_new_null(var->mm));
}

ut *utl_add_bool(ut *var, bool val)
{
	return utl_add(var, ut_new_bool(val, var->mm));
}

ut *utl_add_int(ut *var, int val)
{
	return utl_add(var, ut_new_int(val, var->mm));
}

ut *utl_add_double(ut *var, double val)
{
	return utl_add(var, ut_new_double(val, var->mm));
}

ut *utl_add_char(ut *var, const char *val)
{
	return utl_add(var, ut_new_char(val, var->mm));
}

ut *utl_add_xstr(ut *var, xstr *val)
{
	return utl_add(var, ut_new_xstr(val, var->mm));
}

ut *utl_add_tlist(ut *var, tlist *val)
{
	return utl_add(var, ut_new_tlist(val, var->mm));
}

ut *utl_add_thash(ut *var, thash *val)
{
	return utl_add(var, ut_new_thash(val, var->mm));
}

ut *utl_add_ptr(ut *var, void *ptr)
{
	return utl_add(var, ut_new_ptr(ptr, var->mm));
}
