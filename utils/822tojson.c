/*
 * Copyright (C) 2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pjf@asn.pl>
 *
 * All rights reserved
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
