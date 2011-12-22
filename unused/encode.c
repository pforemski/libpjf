/*
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * Copyright (C) 2010 ASN Sp. z o.o.
 * Author: Pawel Foremski <pawel@foremski.pl>
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "lib.h"

xstr *pjf_b64_dec(const char *text, void *mm)
{
	static const char d[] = {
		62, -1, -1, -1, 63, 52, 53, 54,
		55, 56, 57, 58, 59, 60, 61, -1,
		-1, -1, -2, -1, -1, -1,  0,  1,
		 2,  3,  4,  5,  6,  7,  8,  9,
		10, 11, 12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23, 24, 25,
		-1, -1, -1, -1, -1, -1, 26, 27,
		28, 29, 30, 31, 32, 33, 34, 35,
		36, 37, 38, 39, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51 };

	int m = 0;
	unsigned char b, c;
	xstr *xs = MMXSTR_CREATE("");

	for (; *text >= 43; text++) {
		c = d[(*text - 43) % sizeof(d)];
		if (c > 63)
			continue;

		switch (m) {
			case 0:
				b = c << 2;
				m++; break;
			case 1:
				xstr_append_char(xs, b | (c >> 4));
				b = c << 4;
				m++; break;
			case 2:
				xstr_append_char(xs, b | (c >> 2));
				b = c << 6;
				m++; break;
			case 3:
				xstr_append_char(xs, b | c);
				b = 0;
				m = 0; break;
		}
	}

	return xs;
}

const char *pjf_b64_enc(xstr *text, void *mm)
{
	static const char d[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	xstr *xs = MMXSTR_CREATE("");

	int i, m = 0;
	unsigned char b, c;
	char *s;

	for (i = 0, s = xstr_string(text); i < xstr_length(text); i++) {
		c = s[i];

		switch (m) {
			case 0:
				xstr_append_char(xs, d[c >> 2]);
				b = (c & 3) << 4;
				m++; break;
			case 1:
				xstr_append_char(xs, d[b | c >> 4]);
				b = (c & 15) << 2;
				m++; break;
			case 2:
				xstr_append_char(xs, d[b | c >> 6]);
				xstr_append_char(xs, d[c & 63]);
				m = 0; break;
		}
	}

	if (m > 0) {
		xstr_append_char(xs, d[b]);
		while (m++ < 3) xstr_append_char(xs, '=');
	}

	return xstr_string(xs);
}

xstr *pjf_b32_dec(const char *text, void *mm)
{
	static const unsigned char d[] = {
		  0,   1,   2,   3,   4,   5,   6,   7,   8,   9, // 9
		255, 255, 255, 255, 255, 255, 255,  10,  11,  12, // C
		 13,  14,  15,  16,  17,   1,  18,  19,   1,  20, // M
		 21,   0,  22,  23,  24,  25,  26, 255,  27,  28, // W
		 29,  30,  31, 255, 255, 255, 255, 255, 255,  10, // a
		 11,  12,  13,  14,  15,  16,  17,   1,  18,  19, // k
		  1,  20,  21,   0,  22,  23,  24,  25,  26, 255, // u
		 27,  28,  29,  30,  31
	};

	int m = 0;
	unsigned char b, c;
	xstr *xs = MMXSTR_CREATE("");

	for (; *text >= 48; text++) {
		c = d[(*text - 48) % sizeof(d)];
		if (c == 255)
			return NULL;

		switch (m) {
			case 0:
				b = c << 3;
				m++; break;
			case 1:
				xstr_append_char(xs, b | (c >> 2));
				b = c << 6;
				m++; break;
			case 2:
				b |= c << 1;
				m++; break;
			case 3:
				xstr_append_char(xs, b | (c >> 4));
				b = c << 4;
				m++; break;
			case 4:
				xstr_append_char(xs, b | (c >> 1));
				b = c << 7;
				m++; break;
			case 5:
				b |= c << 2;
				m++; break;
			case 6:
				xstr_append_char(xs, b | (c >> 3));
				b = c << 5;
				m++; break;
			case 7:
				xstr_append_char(xs, b | c);
				b = 0;
				m = 0; break;
		}
	}

	if (m > 0)
		xstr_append_char(xs, b);

	return xs;
}

const char *pjf_b32_enc(xstr *text, void *mm)
{
	static const char d[] = {
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
		'G', 'H', 'J', 'K', 'M', 'N', 'P', 'Q',
		'R', 'S', 'T', 'V', 'W', 'X', 'Y', 'Z'
	};

	xstr *xs = MMXSTR_CREATE("");

	int i, m = 0;
	unsigned char b, c;
	char *s;

	for (i = 0, s = xstr_string(text); i < xstr_length(text); i++) {
		c = s[i];

		switch (m) {
			case 0:
				xstr_append_char(xs, d[c >> 3]);
				b = (c & 7) << 2;
				m++; break;
			case 1:
				xstr_append_char(xs, d[b | c >> 6]);
				b = c & 63;

				xstr_append_char(xs, d[b >> 1]);
				b = (b & 1) << 4;

				m++; break;
			case 2:
				xstr_append_char(xs, d[b | c >> 4]);
				b = (c & 15) << 1;
				m++; break;
			case 3:
				xstr_append_char(xs, d[b | c >> 7]);
				b = c & 127;

				xstr_append_char(xs, d[b >> 2]);
				b = (b & 3) << 3;

				m++; break;
			case 4:
				xstr_append_char(xs, d[b | c >> 5]);
				b = c & 31;

				xstr_append_char(xs, d[b]);
				b = 0;

				m = 0; break;
		}
	}

	if (m > 0)
		xstr_append_char(xs, d[b]);

	return xstr_string(xs);
}
