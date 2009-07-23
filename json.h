/*
 * This file is part of libasn
 * Copyright (C) 2008 Ivo van Poorten
 * Copyright (C) 2009 ASN Sp. z o.o.
 *
 * License: GNU Lesser General Public License version 2.1
 * Imported from zzjson library by: Pawel Foremski <pjf@asn.pl>
 */

#ifndef _JSON_H_
#define _JSON_H_

typedef struct json {
	mmatic *mm;
	int depth;          /** recurrency depth */

	const char *txt;    /** text to parse */
	int i;              /** position in txt */
} json;

/** Create json parser */
json *json_create(mmatic *mm);

/** Parse given string into unitype node */
ut *json_parse(json *j, const char *txt);

#endif
