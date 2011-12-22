/*
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Authors: Dawid Ciężarkiewicz <dawid.ciezarkiewicz@gmail.com> (original idea)
 *          Łukasz Zemczak <sil2100@asn.pl>
 *          Pawel Foremski <pawel@foremski.pl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lib.h"

#define xmmatic_alloc(mm, size) (mmatic_alloc(xs, (size)))

/**
 * @file xstr.c
 * Implement string-like structure to allow easy
 * appending.
 */

xstr *xstr_create(const char *str, void *mm)
{
	xstr *new = mmatic_alloc(mm, sizeof(xstr));
	xstr_init_val(new, (str) ? str : "", mm);
	return new;
}

void xstr_init(xstr *sx, void *mm)
{
	xstr_init_val(sx, "", mm);
}

void xstr_init_val(xstr *sx, const char *ch, void *mm)
{
	sx->s = 0;
	sx->len = 0;
	sx->a = 0;
	xstr_set(sx, ch);
}

char *xstr_to_char(xstr *sx)
{
	return sx->s;
}

char *xstr_dup(xstr *sx, void *mm)
{
	char *ret;

	ret = mmatic_alloc(mm, sx->len + 1);
	strcpy(ret, sx->s);

	return ret;
}

/** Make sure there is a place for l chars in xstr.
 *
 * This means (l+1) bytes. */
void xstr_reserve(xstr *xs, size_t l)
{
	char *new_str;

	if (xs->a > l)
		return;

	new_str = xmmatic_alloc(mm, l + 1);

	if (xs->s) {
		memcpy(new_str, xs->s, xs->len + 1);
		mmatic_free(xs->s);
		xs->s = new_str;
	} else {
		xs->s = new_str;
		xs->s[0] = 0;
		xs->len = 0;
	}

	xs->a = l;
}

void xstr_append(xstr *sx, const char *s)
{
	int slen;

	if (!s)
		return;

	slen = strlen(s);
	if (!slen)
		return;

	slen += sx->len;
	xstr_reserve(sx, slen);
	strcat(sx->s, s);
	sx->len = slen;
}

void xstr_append_size(xstr *sx, const char *s, int size)
{
	int slen;

	if (!s || size == 0)
		return;

	slen = sx->len + size;
	xstr_reserve(sx, slen);

	memcpy(sx->s + sx->len, s, size);
	sx->len = slen;
	sx->s[sx->len] = '\0';
}

void xstr_append_char(xstr *sx, char s)
{
	/* hack to overcome relocating memory sequential calls (eg. loops) */
	if (sx->len + 2 >= sx->a);
		xstr_reserve(sx, MAX(8, 2 * sx->len));

	sx->s[sx->len++] = s;
	sx->s[sx->len] = 0;
}

void xstr_set(xstr *xs, const char *s)
{
	int l = strlen(s);

	xstr_reserve(xs, l);
	strcpy(xs->s, s);
	xs->len = l;
}

void xstr_set_size(xstr *xs, const char *s, int size)
{
	xstr_reserve(xs, size);
	memcpy(xs->s, s, size);
	xs->len = size;
	xs->s[size] = '\0';
}

void xstr_free(xstr *xs)
{
	if (xs->s) {
		mmatic_free(xs->s);
		xs->s = 0;
		xs->len = 0;
		xs->a = 0;
	}
}

char *xstr_strip(xstr *xs)
{
	char *ret;
	int i,j;
	int b;

	for (i = 0; i < xs->len; ++i) if (isgraph(xs->s[i])) break;
	for (j = xs->len; j >= i; --j) if (isgraph(xs->s[j])) break;

	ret = xmmatic_alloc(mm, sizeof(char) * (j-i + 2));
	for (b = 0; i < j+1; i++, b++) ret[b] = xs->s[i];
	ret[b] = 0;

	return ret;
}

char *xstr_stripch(char *string, void *mm)
{
	xstr xs;
	char *s;

	xstr_init(&xs, mm);
	xstr_set(&xs, string);
	s = xstr_strip(&xs);
	xstr_free(&xs);

	return s;
}

/* XXX: The following might not work on other standards than C99, due to some
 *      snprintf inconsistency between standards */

int xstr_set_format(xstr *xs, const char *format, ...)
{
	int len;
	va_list args;

	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args);
	xstr_reserve(xs, len);
	if (vsnprintf(xs->s, xs->a + 1, format, args) != len) len = -1;
	va_end(args);

	if (len > 0) xs->len = len;

	return len;
}

int xstr_append_format(xstr *xs, const char *format, ...)
{
	char *ptr;
	int len;
	va_list args;

	va_start(args, format);
	len = vsnprintf(NULL, 0, format, args);
	xstr_reserve(xs, xs->len + len);
	ptr = xs->s + xs->len;
	if (vsnprintf(ptr, xs->a - xs->len + 1, format, args) != len) len = -1;
	va_end(args);

	if (len > 0) xs->len += len;

	return len;
}

void xstr_cut(xstr *xs, unsigned int l)
{
	if (l > xs->len) return;
	xs->len -= l;
	xs->s[xs->len] = '\0';
}
