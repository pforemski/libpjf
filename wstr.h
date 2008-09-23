/*
 * Wide character version of the libasn xstr
 *
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
 * Author: Łukasz Zemczak <sil2100@asn.pl>
 *
 * libkinput is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * libkinput is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WSTR_H_
#define __WSTR_H_

#include "mmatic.h"

#include <wchar.h>

typedef struct wstr_t {
	/** The actual wide string */
	wchar_t *s;

	/** Length of the wide string (in wchar elements) */
	size_t len;

	/** Allocated buffer size (in wchar's) */
	size_t a;

	/** mmatic object */
	mmatic *mm;
} wstr;

/** 
 * @file wstr.h
 * All functions have their equivalents in xstr of the libasn library, only
 * handling wide strings and wide characters. Consult the xstr.h header
 * documentation for function description
 */

void wstr_init(wstr *ws, mmatic *mm);
wstr * wstr_create(mmatic *mm);
void wstr_reserve(wstr *ws, size_t l);
void wstr_append(wstr *ws, const wchar_t *s);
void wstr_append_size(wstr *ws, const wchar_t *s, size_t size);
void wstr_append_char(wstr *ws, wchar_t s);
void wstr_set(wstr *ws, const wchar_t *s);
void wstr_set_size(wstr *ws, const wchar_t *s, size_t size);
void wstr_insert_char(wstr *ws, int32_t pos, wchar_t s);
void wstr_remove_char(wstr *ws, int32_t pos);
void wstr_free(wstr *ws);
wchar_t * wstr_dup(wstr *ws, mmatic *mm);

#endif
