/*
 * This file is part of libpjf
 * Copyright (C) 2010 ASN Sp. z o.o.
 * Author: Pawel Foremski <pforemski@asn.pl>
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

#define HEX2VAL(l) do { l =                \
	(l >= 'A' && l <= 'F' ? l - 'A' + 10 : \
	(l >= 'a' && l <= 'f' ? l - 'a' + 10:  \
	(l >= '0' && l <= '9' ? l - '0' :      \
	0xff)));                               \
	if (l == 0xff) goto err;               \
} while(0);

/* TODO: this was written in a hurry */
void utf8_parse_xcp(xstr *str, char l1, char l2, char l3, char l4)
{
	uint32_t cp;

	HEX2VAL(l1);
	HEX2VAL(l2);
	HEX2VAL(l3);
	HEX2VAL(l4);

	cp = l1 << 12 | l2 << 8 | l3 << 4 | l4;
	if (cp <= 0x7f) {
		xstr_append_char(str, (uint8_t) cp);
	} else if (cp <= 0x7ff) {
		xstr_append_char(str, (uint8_t) 0xc0 | (cp >> 6));
		xstr_append_char(str, (uint8_t) 0x80 | (cp & 0x3f));
	} else {
		xstr_append_char(str, (uint8_t) 0xe0 | (cp >> 12));
		xstr_append_char(str, (uint8_t) 0x80 | ((cp >> 6) & 0x3f));
		xstr_append_char(str, (uint8_t) 0x80 | (cp & 0x3f));
	}

	return;

err:
	xstr_append_char(str, '?');
}
