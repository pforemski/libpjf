/*
 * Copyright (C) 2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pjf@asn.pl>
 *
 * All rights reserved
 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include "lib.h"

__USE_LIBASN
mmatic *mm;

int main(int argc, char *argv[])
{
	printf("%s", rfc822_print(asn_ipa((argc > 1), mmatic_create())));
	return 0;
}
