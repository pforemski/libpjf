/*
 * tsort - topological sort
 *
 * This file is part of libasn
 * Copyright (C) 1998-2007 Free Software Foundation, Inc.
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
 *
 * Originally written by Mark Kettenis <kettenis@phys.uva.nl>.
 *
 * tsort is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * Foobar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libasn/thash.h>
#include <libasn/tlist.h>
#include <libasn/mmatic.h>

/** A structure representing a dependency relation */
typedef struct _tsort_pair {
	/** What... */
	char *what;

	/** ...depends on this */
	char *dependson;
} tsort_pair;

/** Do a topological sort on input
 *
 * @param  input  a tlist of struct tsort_pair elements
 * @param  output an already initialized tlist to push() results to
 * @return 1 if successful
 * @return 0 otherwise
 */
int tsort(tlist *input, tlist *output, mmatic *mm);
