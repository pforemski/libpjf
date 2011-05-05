/*
 * 822tojson - a RFC822 to JSON translator
 *
 * This file is part of libasn
 * Copyright (C) 2009-2010 ASN Sp. z o.o.
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>

#include "lib.h"

__USE_LIBASN

mmatic *mm;

int main(int argc, char *argv[])
{
	char buf[BUFSIZ], *ret;

	setvbuf(stdin, 0, _IOLBF, 0);
	setvbuf(stdout, 0, _IOLBF, 0);
	setvbuf(stderr, 0, _IOLBF, 0);

	mmatic *mm = mmatic_create();

	xstr *xs = xstr_create("", mm);
	json *js = json_create(mm);

	do {
		while ((ret = fgets(buf, sizeof(buf), stdin))) {
			if (!buf[0] || buf[0] == '\n')
				break;

			xstr_append(xs, buf);
		}

		fputs(
			json_print(js, ut_new_thash(
				rfc822_parse(xstr_string(xs), mm), mm)), stdout);
		fputc('\n', stdout);

		xstr_set(xs, "");
	} while (ret);

	return 1;
}

/* for Vim autocompletion:
 * vim: path=.,/usr/include,/usr/local/include,~/local/include
 */
