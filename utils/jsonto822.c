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
	char buf[8 * BUFSIZ], *ret;

	setvbuf(stdin, 0, _IOLBF, 0);
	setvbuf(stdout, 0, _IOLBF, 0);
	setvbuf(stderr, 0, _IOLBF, 0);

	mmatic *mm = mmatic_create();

	json *js = json_create(mm);

	do {
		ret = fgets(buf, sizeof(buf), stdin);

		fputs(
			rfc822_print(
				json_parse(js, buf)), stdout);
		fputc('\n', stdout);

	} while (ret);

	return 1;
}

/* for Vim autocompletion:
 * vim: path=.,/usr/include,/usr/local/include,~/local/include
 */
