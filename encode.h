/*
 * This file is part of libasn
 * Copyright (C) 2010 ASN Sp. z o.o.
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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ENCODE_H_
#define _ENCODE_H_

/** Decode base64 encoded string
 * @note resulting string may contain \0 - check xs->len */
xstr *asn_b64dec(const char *text, mmatic *mm);

/** Decode base32
 * @retval NULL decoding failed
 * @note resulting string may contain \0 - check xs->len
 * @note we use alternative base32 - see http://www.crockford.com/wrmg/base32.html
 */
xstr *asn_b32dec(const char *text, mmatic *mm);

/** Encode base32
 * @note we use alternative base32 - see http://www.crockford.com/wrmg/base32.html
 */
const char *asn_b32enc(xstr *text, mmatic *mm);

#endif
