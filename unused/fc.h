/*
 * fc - Flatconf datatree read-only support
 *
 * This file is part of libpjf
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pjf@asn.pl>
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
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FC_H_
#define _FC_H_

#include "lib.h"

/** Read recursively a Flatconf datatree
 * @return thash - keys are relative paths (e.g. "foo", "foo/bar/woo")
 * @param  path   directory path */
thash *asn_fcdir(const char *path, void *mm);

/** A safe wrapper around thash_get to fetch config values
 * @param hash   configuration hash returned from asn_fcdir()
 * @param path   variable path */
const char *asn_fcget(thash *hash, const char *path);

/** Structure representing a list element */
struct fcel {
	bool enabled;
	unsigned int elid;
	char *elname;
};

/** Return a list of struct fcel representing contents of given list
 * @param  listorder  contents of *.order file */
tlist *asn_fcparselist(const char *listorder, void *mm);

#endif /* _FC_H_ */
