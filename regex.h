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

#ifndef _REGEX_H_
#define _REGEX_H_

/** Simple regex matching
 *
 * @param regex pattern
 * @param str   string
 *
 * @retval  1 matched
 * @retval  0 didn't match
 * @retval -1 pattern error */
int asn_match(char *regex, char *str);

/** Simple regex replace
 *
 * @param regex pattern
 * @param str   subject
 * @param rep   replacement
 * @return      new char *, always succeeds */
char *asn_replace(char *regex, const char *str, char *rep, mmatic *mm);

#endif /* _REGEX_H_ */
