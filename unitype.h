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

/***** check type wrappers *****/

#define ut_is_ptr(ut)    (ut->type == T_PTR)
#define ut_is_bool(ut)   (ut->type == T_BOOL)
#define ut_is_int(ut)    (ut->type == T_INT)
#define ut_is_double(ut) (ut->type == T_DOUBLE)
#define ut_is_string(ut) (ut->type == T_STRING)
#define ut_is_tlist(ut)  (ut->type == T_LIST)
#define ut_is_thash(ut)  (ut->type == T_HASH)

/***** error handling *****/

/** Checks if ut is not of err type */
#define ut_ok(ut) (ut && ut->type != T_ERR)

/** Returns human-readable error description */
const char *ut_err(ut *ut);

/** Returns error code */
int ut_errcode(ut *ut);

/***** type conversions *****/

/** Return type of variable */
#define ut_type(ut) (ut ? ut->type : T_NULL)

/* XXX: BIG FAT WARNINGS:
 *
 * 1. returned values are read-only
 * 2. if conversion needs to be done, a completely new memory is allocated @ut->mm and the pointer
 *    wont be cached at our side - its your task, if needed
 */

bool        ut_bool(ut *ut);
int         ut_int(ut *ut);
double      ut_double(ut *ut);
xstr       *ut_xstr(ut *ut);
const char *ut_char(ut *ut);
tlist      *ut_tlist(ut *ut);
thash      *ut_thash(ut *ut);
void       *ut_ptr(ut *ut);

/***** create new unitype object - never fail ******/

ut *ut_new_bool(bool val, mmatic *mm);
ut *ut_new_int(int val, mmatic *mm);
ut *ut_new_double(double val, mmatic *mm);
ut *ut_new_char(const char *val, mmatic *mm);
ut *ut_new_xstr(xstr *val, mmatic *mm);
ut *ut_new_ptr(void *val, mmatic *mm);
ut *ut_new_null(mmatic *mm);
ut *ut_new_err(int code, const char *msg, const char *data, mmatic *mm);

/***** hash list *****/

/** Create a ut containing given hash of string->string
 * @param val may be NULL to create a new hash */
ut *ut_new_thash(thash *val, mmatic *mm);

/** Create a ut containing given hash of string->ut objects
 * @param val may be NULL to create a new hash */
ut *ut_new_utthash(thash *val, mmatic *mm);

/* applicable for ut->type == T_HASH */
ut *uth_get(ut *var, const char *key);
ut *uth_set(ut *var, const char *key, ut *val);
ut *uth_set_bool(ut *ut, const char *key, bool val);
ut *uth_set_int(ut *ut, const char *key, int val);
ut *uth_set_double(ut *ut, const char *key, double val);
ut *uth_set_char(ut *ut, const char *key, const char *val);
ut *uth_set_xstr(ut *ut, const char *key, xstr *val);
ut *uth_set_ptr(ut *ut, const char *key, void *ptr);
ut *uth_set_tlist(ut *ut, const char *key, tlist *val); /** @note see ut_new_tlist */
ut *uth_set_thash(ut *ut, const char *key, thash *val); /** @note see ut_new_thash */

/* shortcuts */
#define uth_bool(var, key)   ut_bool(uth_get(var, key))
#define uth_int(var, key)    ut_int(uth_get(var, key))
#define uth_double(var, key) ut_double(uth_get(var, key))
#define uth_xstr(var, key)   ut_xstr(uth_get(var, key))
#define uth_char(var, key)   ut_char(uth_get(var, key))
#define uth_tlist(var, key)  ut_tlist(uth_get(var, key))
#define uth_thash(var, key)  ut_thash(uth_get(var, key))
#define uth_ptr(var, key)    ut_ptr(uth_get(var, key))

/***** linked list *****/

/** Create a ut containing given list of ut objects
 * @param val may be NULL to create a new list */
ut *ut_new_uttlist(tlist *val, mmatic *mm);

/** Create a ut containing given list of strings
 * @param val may be NULL to create a new list */
ut *ut_new_tlist(tlist *val, mmatic *mm);

/* applicable for ut->type == T_LIST */
ut *utl_add(ut *var, ut *val);
ut *utl_add_bool(ut *ut, bool val);
ut *utl_add_int(ut *ut, int val);
ut *utl_add_double(ut *ut, double val);
ut *utl_add_char(ut *ut, const char *val);
ut *utl_add_xstr(ut *ut, xstr *val);
ut *utl_add_ptr(ut *ut, void *ptr);
ut *utl_add_tlist(ut *ut, tlist *val);  /** @note see ut_new_tlist */
ut *utl_add_thash(ut *ut, thash *val);  /** @note see ut_new_thash */

#endif
