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

#include <dirent.h>

#include "misc.h"
#include "thash.h"
#include "mmatic.h"

static void _asn_fcdir(thash *hash, const char *path, mmatic *mm)
{
	struct dirent *dirp;
	DIR *subd;
	char *np, *v;
	uint32_t len;

	if (!(subd = opendir(path)))
		die("opendir(%s) failed: %m\n", path);

	while ((dirp = readdir(subd))) {
		if (dirp->d_name[0] == '.') continue;

		np = (streq(path, ".")) ? dirp->d_name : mmprintf("%s/%s", path, dirp->d_name);
		if (asn_isdir(np) == 1) {
			_asn_fcdir(hash, np, mm);
		}
		else {
			v = asn_readfile(np, mm);
			if (!v) die("%s: could not read config file (%m)\n", np); /* fopen() failed */

			/* trim endlines at the end */
			len = strlen(v);
			if (len && v[len-1] == '\n') v[len-1] = '\0';

			thash_set(hash, np, v);
		}
	}

	closedir(subd);
}

thash *asn_fcdir(const char *path, mmatic *mm)
{
	thash *ret;
	char *pwd;

	if (asn_isdir(path) != 1)
		die("%s: not a directory\n", path);

	pwd = asn_pwd(mm);
	asn_cd(path);

	ret = MMTHASH_CREATE_STR(mmfreeptr);
	_asn_fcdir(ret, ".", mm);

	asn_cd(pwd);
	return ret;
}

const char *asn_fcget(thash *hash, const char *path)
{
	char *value;

	value = (char *) thash_get(hash, path);
	if (value == NULL)
		die("asn_fcget(): could not get the value of '%s'\n", path);

	return value;
}
