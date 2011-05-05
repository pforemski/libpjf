/*
 * This file is part of libpjf
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Authors: Dawid Ciężarkiewicz <dawid.ciezarkiewicz@gmail.com> (original pjf_match)
 *          Pawel Foremski <pawel@foremski.pl>
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

#ifndef _REGEX_H_
#define _REGEX_H_

/** Simple regex matching
 *
 * @param regex pattern enclosed in delimeters, with optional flags
 *              example: /^fooBAR$/i -- searches for ^fooBAR$ in caseless mode
 *              supported flags: i, m, s, x, A, D, U, X (see PHP preg_match)
 * @param str   string
 *
 * @retval  1 matched
 * @retval  0 didn't match
 * @retval -1 pattern error */
int pjf_match(const char *regex, const char *str);

/** Simple regex replace
 *
 * @param regex pattern (see pjf_match())
 * @param str   subject
 * @param rep   replacement (supports back-references)
 * @return      new char *, always succeeds */
char *pjf_replace(const char *regex, const char *rep, const char *str, void *mm);

#endif /* _REGEX_H_ */
