/*
 * Wide character version of the libpjf xstr
 *
 * This file is part of libpjf
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
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

#include <string.h>
#include "lib.h"

#define wmmalloc(size) (mmatic_alloc((size), ws))

void wstr_init(wstr *ws, void *mm)
{
	ws->s = 0;
	ws->len = 0;
	ws->a = 0;
	wstr_reserve(ws, 20);
	wstr_set(ws, L"");
}

wstr * wstr_create(const wchar_t *s, void *mm)
{
	wstr *new = mmalloc(sizeof(wstr));
	wstr_init(new, mm);
	wstr_set(new, s);
	return new;
}

void wstr_reserve(wstr *ws, size_t l)
{
	wchar_t *new_str;

	if (ws->a > l)
		return;

	new_str = wmmalloc(sizeof(wchar_t) * (l + 1));

	if (ws->s) {
		memcpy(new_str, ws->s, sizeof(wchar_t) * (ws->len + 1));
		mmfreeptr(ws->s);
		ws->s = new_str;
	} else {
		ws->s = new_str;
		ws->s[0] = 0;
		ws->len = 0;
	}

	ws->a = l;
}

void wstr_append(wstr *ws, const wchar_t *s)
{
	int slen;

	if (!s)
		return;

	slen = wcslen(s) + ws->len;
	wstr_reserve(ws, slen);
	wcscat(ws->s, s);
	ws->len = slen;
}

void wstr_append_size(wstr *ws, const wchar_t *s, size_t size)
{
	size_t slen;

	if (!s)
		return;

	slen = ws->len + size;
	wstr_reserve(ws, slen);
	wcsncat(ws->s, s, size);
	ws->len = slen;
}

void wstr_append_char(wstr *ws, wchar_t s)
{
	wstr_reserve(ws, ws->len + 1);
	ws->s[ws->len] = s;
	ws->len++;
	ws->s[ws->len] = 0;
}

void wstr_insert_char(wstr *ws, int32_t pos, wchar_t s)
{
	wchar_t *ptr;

	if (pos >= ws->len) {
		wstr_append_char(ws, s);
		return;
	}

	wstr_reserve(ws, ws->len + 1);
	ptr = &(ws->s[pos]);
	memmove(ptr + 1, ptr, sizeof(wchar_t) * wcslen(ptr));
	ws->s[pos] = s;
	ws->len++;
	ws->s[ws->len] = 0;
}

void wstr_remove_char(wstr *ws, int32_t pos)
{
	wchar_t *ptr;
	
	if (pos < 0 || pos > ws->len)
		return;
	
	ptr = &(ws->s[pos]);
	memmove(ptr, ptr + 1, sizeof(wchar_t) * wcslen(ptr));
	ws->len--;
}

void wstr_set(wstr *ws, const wchar_t *s)
{
	int l = wcslen(s);

	wstr_reserve(ws, l);
	wcscpy(ws->s, s);
	ws->len = l;
}

void wstr_set_size(wstr *ws, const wchar_t *s, size_t size)
{
	wstr_reserve(ws, size);
	wcsncpy(ws->s, s, size);
	ws->len = size;
}

void wstr_free(wstr *ws)
{
	if (ws->s) {
		mmfreeptr(ws->s);
		ws->s = 0;
		ws->len = 0;
		ws->a = 0;
	}
}

#define DUP_STRING(str, len) \
	wchar_t *ret = mmalloc((len + 1) * sizeof(wchar_t)); \
	wcscpy(ret, str); \
	return ret;

wchar_t * wstr_dup(wstr *ws, void *mm)
{
	DUP_STRING(ws->s, ws->len);
}

wchar_t * wstr_dup_ch(const wchar_t *s, void *mm)
{
	DUP_STRING(s, wcslen(s));
}

#undef DUP_STRING
