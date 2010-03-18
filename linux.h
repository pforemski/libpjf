/*
 * linux - various utilities specific for Linux
 *
 * This file is part of libasn
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LINUX_H_
#define _LINUX_H_

#include "lib.h"

/** Fetch all IPv4 addresses on all interfaces
 * @param index_by_ip   if false, returns IPs indexed by interfaces, swapped otherwise
 * @retval NULL         request failed, error message sent to dbg() */
ut *asn_ipa(bool index_by_ip, mmatic *mm);

#endif /* _LINUX_H_ */
