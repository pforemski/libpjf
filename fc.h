/*
 * fc - Flatconf datatree read-only support
 *
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
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

#ifndef _FC_H_
#define _FC_H_

#include "lib.h"

/** Read recursively a Flatconf datatree
 * @return thash - keys are relative paths (e.g. "foo", "foo/bar/woo")
 * @param  path   directory path */
thash *asn_fcdir(const char *path, mmatic *mm);

/** A safe wrapper around thash_get to fetch config values
 * @param hash   configuration hash
 * @param path   variable path */
const char *asn_fcget(thash *hash, const char *path);

#endif /* _FC_H_ */
