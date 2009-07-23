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

#ifndef _UNITYPE_H_
#define _UNITYPE_H_

#include "lib.h"

typedef struct ut {
	mmatic *mm;

	enum ut_type {
		T_PTR,       /* void*  */
		T_BOOL,      /* bool   */
		T_INT,       /* int    */
		T_DOUBLE,    /* double */
		T_STRING,    /* xstr   */
		T_LIST,      /* tlist  */
		T_HASH,      /* thash string->ut */

		/* special types */
		T_NULL,
		T_ERR,
	} type;

	union ut_as {
		bool        as_bool;
		int         as_int;
		double      as_double;
		xstr       *as_xstr;
		tlist      *as_tlist;
		thash      *as_thash;
		void       *as_ptr;

		struct ut_err {
			int  code;
			const char *msg;  /** XXX: never null */
			const char *data; /** XXX: may be null */
		} *as_err;
	} d;
} ut;

/** Return type of variable */
enum ut_type ut_type(ut *ut);

/** Checks if ut is not of err type */
#define ut_ok(ut) (ut->type != T_ERR)

/** Returns human-readable error description */
const char *ut_err(ut *ut);

/* big fat warning: if conversion needs to be done, a completely new memory is
 * allocated @ut->mm and it wont be referenced at our side - its your task if required */
bool        ut_bool(ut *ut);
int         ut_int(ut *ut);
double      ut_double(ut *ut);
xstr       *ut_xstr(ut *ut);
const char *ut_char(ut *ut);
tlist      *ut_tlist(ut *ut);
thash      *ut_thash(ut *ut);
void       *ut_ptr(ut *ut);

ut *ut_new_bool(bool val, mmatic *mm);
ut *ut_new_int(int val, mmatic *mm);
ut *ut_new_double(double val, mmatic *mm);
ut *ut_new_char(const char *val, mmatic *mm);
ut *ut_new_xstr(xstr *val, mmatic *mm);
ut *ut_new_tlist(tlist *val, mmatic *mm);
ut *ut_new_thash(thash *val, mmatic *mm);
ut *ut_new_ptr(void *val, mmatic *mm);
ut *ut_new_null(mmatic *mm);
ut *ut_new_err(int code, const char *msg, const char *data, mmatic *mm);

/* applicable for ut->type == T_HASH */
ut *uth_add_ut(ut *var, const char *key, ut *val);
ut *uth_add_bool(ut *ut, const char *key, bool val);
ut *uth_add_int(ut *ut, const char *key, int val);
ut *uth_add_double(ut *ut, const char *key, double val);
ut *uth_add_char(ut *ut, const char *key, const char *val);
ut *uth_add_xstr(ut *ut, const char *key, xstr *val);
ut *uth_add_tlist(ut *ut, const char *key, tlist *val);
ut *uth_add_thash(ut *ut, const char *key, thash *val);
ut *uth_add_ptr(ut *ut, const char *key, void *ptr);

/* applicable for ut->type == T_LIST */
ut *utl_add_ut(ut *var, ut *val);
ut *utl_add_bool(ut *ut, bool val);
ut *utl_add_int(ut *ut, int val);
ut *utl_add_double(ut *ut, double val);
ut *utl_add_char(ut *ut, const char *val);
ut *utl_add_xstr(ut *ut, xstr *val);
ut *utl_add_tlist(ut *ut, tlist *val);
ut *utl_add_thash(ut *ut, thash *val);
ut *utl_add_ptr(ut *ut, void *ptr);

#endif
