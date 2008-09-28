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

#include <pcre.h>
#include <ctype.h>
#include <string.h>

#include "misc.h"
#include "xstr.h"
#include "mmatic.h"

#define CVS 90

static int _regex_match(char *regex, char *str, int cv[CVS], int *cvn)
{
	pcre *re;
	int rc, mods = 0;
	char *pattern;
	char *pp = regex;
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
	rc = pcre_exec(re, 0, str, strlen(str), 0, 0, cv, CVS);
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

int asn_match(char *regex, char *str) { int cv[CVS]; return _regex_match(regex, str, cv, NULL); }

char *asn_replace(char *regex, char *str, char *rep, mmatic *mm)
{
	int cv[CVS], cvn, rc, done = 0;
	xstr *xs = MMXSTR_CREATE("");

	while ((rc = _regex_match(regex, str, cv, &cvn)) == 1 && cv[0] > 0 && cv[1] > cv[0]) {
		xstr_append_size(xs, str, cv[0]);  /* up to first match */
		xstr_append(xs, rep);              /* replace */
		str += cv[1];                      /* start next match after */
		done++;
	}
	xstr_append(xs, str);                  /* end of string, with no matches (may be just "") */

	if (rc == 1) dbg(0, "regex_replace(): this should not happen\n");
	return xs->s;
}
