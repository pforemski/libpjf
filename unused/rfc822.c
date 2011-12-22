/*
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
 * Copyright (C) 2009 ASN Sp. z o.o.
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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lib.h"

thash *rfc822_parse(const char *str, void *mm)
{
	char *k, *v, *txt = mmatic_strdup(mm, str);
	int i, l;
	thash *ret =  thash_create_strkey(NULL, mm);

	txt = pjf_trim(txt);
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
			while (txt[i] && txt[i] != '\n' && txt[i] != '\r') i++;
			if (strncmp(txt+i, "\n ", 2) == 0) { i += 2; goto nextline; }
			else if (strncmp(txt+i, "\r\n ", 3) == 0) { i += 3; goto nextline; }
			txt[i] = '\0';
		}

		thash_set(ret, k, v);
	}

	return ret;
}

const char *rfc822_print(ut *var)
{
	void *mm = var;
	xstr *xs = MMXSTR_CREATE("");
	char *key;
	ut *el;

	if (var->type != T_HASH)
		return ut_char(var);

	thash_iter_loop(var->d.as_thash, key, el) {
		xstr_append(xs, key);
		xstr_append(xs, ": ");
		xstr_append(xs, ut_char(el));
		xstr_append_char(xs, '\n');
	}

	return xstr_string(xs);
}
