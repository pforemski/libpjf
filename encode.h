/*
 * This file is part of libpjf
 * Copyright (C) 2010 ASN Sp. z o.o.
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

#ifndef _ENCODE_H_
#define _ENCODE_H_

/** Decode base64 encoded string
 * @note resulting string may contain \0 - check xs->len */
xstr *pjf_b64_dec(const char *text, void *mm);

/** Encode base64 encoded string
 * @note we dont pad with '=' */
const char *pjf_b64_enc(xstr *text, void *mm);

/** Decode base32
 * @retval NULL decoding failed
 * @note resulting string may contain \0 - check xs->len
 * @note we use alternative base32 - see http://www.crockford.com/wrmg/base32.html
 */
xstr *pjf_b32_dec(const char *text, void *mm);

/** Encode base32
 * @note we use alternative base32 - see http://www.crockford.com/wrmg/base32.html
 */
const char *pjf_b32_enc(xstr *text, void *mm);

#endif
