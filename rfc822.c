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

thash *rfc822_parse(const char *str, mmatic *mm)
{
	char *k, *v, *txt = mmstrdup(str);
	int i, l;
	thash *ret =  MMTHASH_CREATE_STR(NULL);

	txt = asn_trim(txt);
	l = strlen(txt);

	for (i = 0; i < l; i++) {
		while (txt[i] && isspace(txt[i])) i++;
		k = txt + i;

		while (txt[i] && txt[i] != ':') i++;
		if (txt[i] != ':') break;
		txt[i++] = '\0';

		while (txt[i] && txt[i] == ' ') i++;
		if (!txt[i]) {
			v = "";
		}
		else {
			v = txt + i;

nextline:
			while (txt[i] && txt[i] != '\n') i++;
			if (txt[i] == '\n' && txt[i+1] == ' ') { i += 2; goto nextline; }
			txt[i] = '\0';
		}

		thash_set(ret, k, v);
	}

	return ret;
}
