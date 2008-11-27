/*
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
 * Authors: Dawid Ciężarkiewicz <dawid.ciezarkiewicz@gmail.com> (original asn_match)
 *          Pawel Foremski <pjf@asn.pl>
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

#define _GNU_SOURCE 1

#include <ctype.h>
#include <string.h>

#include "lib.h"
#include "pcre/pcre.h"

#define CVS 90

static int _regex_match(const char *regex, const char *str, int len, int offset, int cv[CVS], int *cvn)
{
	pcre *re;
	int rc, mods = 0;
	char *pattern;
	const char *pp = regex;
	char delim = regex[0];
	const char *err;

	/* read the pattern */
	if (isalnum(delim) || delim == '\\') {
		dbg(8, "regex_match(): no alphanumeric nor '\\' character allowed as delimiter\n");
		return -1;
	}

	while (*(++pp))
		if (pp[0] == delim && pp[-1] != '\\') break;

	if (!pp[0]) {
		dbg(8, "regex_match(): no ending delimiter found\n");
		return -1;
	}

	pattern = strndup(regex + 1, pp - regex - 1);

	/* analyze the flags */
	while (*(++pp)) switch (pp[0]) {
		case 'i': mods |= PCRE_CASELESS; break;
		case 'm': mods |= PCRE_MULTILINE; break;
		case 's': mods |= PCRE_DOTALL; break;
		case 'x': mods |= PCRE_EXTENDED; break;
		case 'A': mods |= PCRE_ANCHORED; break;
		case 'D': mods |= PCRE_DOLLAR_ENDONLY; break;
		case 'U': mods |= PCRE_UNGREEDY; break;
		case 'X': mods |= PCRE_EXTRA; break;
		default: break; /* XXX: fallback silently */
	}

	/* compile the pattern */
	dbg(10, "regex_match(): compiling '%s'\n", pattern);
	re = pcre_compile(pattern, mods, &err, &rc, 0);
	free(pattern);
	if (!re) {
		dbg(3, "regex_match(): pcre_compile(): character %d: %s\n", rc, err);
		return -1;
	}

	/* run it */
	dbg(10, "regex_match(): matching '%s'\n", str);
	rc = pcre_exec(re, 0, str, len, offset, 0, cv, CVS);
	if (cvn) *cvn = rc;

	/* translate the return code */
	if (rc > 0)
		rc = 1;
	else if (rc == PCRE_ERROR_NOMATCH)
		rc = 0;
	else if (rc == 0)
		dbg(5, "regex_match(): cv not big enough\n");
	else {
		dbg(8, "regex_match(): pcre_exec() failed with error code %d\n", rc);
		rc = -1;
	}

	pcre_free(re);
	return rc;
}

int asn_match(const char *regex, const char *str)
{
	int cv[CVS];

	return _regex_match(regex, str, strlen(str), 0, cv, NULL);
}

char *asn_replace(const char *regex, const char *rep, const char *str, mmatic *mm)
{
	int cv[CVS], cvn, rc, br, offset = 0, len = strlen(str);
	xstr *xs = MMXSTR_CREATE("");
	char *mem = malloc(strlen(rep) + 1), *p, *bs;

	while ((rc = _regex_match(regex, str, len, offset, cv, &cvn)) == 1 && cv[0] >= 0 && cv[1] >= cv[0]) {
		dbg(8, "asn_replace(): matched at %d (rc=%d, offset=%d)\n", cv[0], rc, offset);

		/* copy text up to the first match */
		xstr_append_size(xs, str+offset, cv[0]-offset);

		/* replace, handling backreferences */
		strcpy(mem, rep);
		for (bs = p = mem; (bs = strchr(bs, '\\'));) {
			if (!bs[1] || !isdigit(bs[1])) continue;

			/* append everything up to \, position bs on the number */
			*bs++ = '\0';
			xstr_append(xs, p);

			/* position p on the end of the number + 1 */
			for (p = bs; *p && isdigit(*p); p++);

			/* substitute */
			br = atoi(bs);
			if (br++ > 0 && br <= cvn) {
#				define IB (2*br - 2)
#				define IT (2*br - 1)
				dbg(9, "asn_replace(): appending backreference %d between %d and %d\n", br-1, cv[IB], cv[IT]-1);
				xstr_append_size(xs, str + cv[IB], cv[IT] - cv[IB]);
			}
			else {
				dbg(1, "asn_replace(): invalid backreference: %d\n", br-1);
			}

			bs = p;
		}

		/* in no backreferences case, this appends the whole "rep" string */
		xstr_append(xs, p);

		offset = cv[1];             /* start next match after */
	}

	asnsert(offset <= len);
	xstr_append(xs, str + offset);  /* may be just "" */

	if (rc == 1) dbg(0, "regex_replace(): this should not happen\n");

	free(mem);
	return xs->s;
}
