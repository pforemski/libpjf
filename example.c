/*
 * example - an exemplary application
 *
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
 * Authors: Łukasz Zemczak <sil2100@asn.pl>
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

#include "lib.h"

#include <stdio.h>
#include <errno.h>

/* We use this macro to indicate that we will be using libasn. What this does
 * is, in fact, defining the debug variable (debug level, right now equal to 0)
 * and setting the debugcb pointer to NULL, indicating to use standard
 * fprintf to stderr for debugging messages */
__USE_LIBASN

static void usage(void)
{
	fprintf(stdout, "Usage: example debug_level <key>:<value> [<key>:<value> ...]\n");
}

int main(int argc, char **argv)
{
	int i;
	char *pair, *ptr, *key, *value;
	xstr *str;
	thash *hash;
	tlist *list;
	mmatic *mm;

	if (argc < 3) {
		usage();
		return 0;
	}

	debug = strtol(argv[1], NULL, 10);
	if (errno == ERANGE || errno == EINVAL) {
		fprintf(stdout, "Invalid debug level given.\n\n");
		usage();
		return -1;
	}

	/* We create a mmatic object, that will be used to allocate memory later on */
	mm = mmatic_create();

	/* Create a list object using the mmatic instance created previously */
	list = tlist_create(NULL, mm);
	dbg(1, "main(): Creating and filling our tlist\n");

	/* Let's browse through the arguments list */
	for (i = 2; i < argc; i++) {
		/* Add all arguments matching our pattern to the list we created */
		if (asn_match("*:*", argv[i])) {
			dbg(2, "main(): We have a match to our regexp: %s\n", argv[i]);
			tlist_push(list, argv[i]);
		}
	}

	/* Let's create a hash array, to which we will add our key:value pairs.
	 * This will be a string hash table, where the keys are strings. We will use
	 * the default string hash function - that's why we are using an useful
	 * macro to set all the necessary arguments as they should. We set the
	 * mmfreeptr() function as the values free function.
	 * mmfreeptr() is a standard free function for memory previously allocated
	 * using mmalloc() or any other mmatic allocations */
	hash = MMTHASH_CREATE_STR(mmfreeptr);

	/* We will also use a extended string, which we will be using soon as a
	 * temporary buffer */
	str = xstr_create("", mm);

	/* Now, let's parse the key:value pairs and input them to the hash table */
	tlist_reset(list);
	i = 0;
	while ((pair = (char *)tlist_iter(list)) != NULL) {
		ptr = strchr(pair, ':');
		/* We first copy the key string to our xstr, as a temporary placeholder */
		xstr_set_size(str, pair, (size_t)(ptr - pair));
		/* We now duplicate the string using mmallocated standard string */
		key = xstr_dup(str, mm);

		/* The same with the value, but with the difference, that we want the 
		 * element's index at the beginning of the value string */
		xstr_set_format(str, "%d:", i);
		/* We could use xstr_set_format() to add this too, but as a demo we will
		 * use the xstr_append() function instead */
		xstr_append(str, ptr + 1);
		value = xstr_dup(str, mm);

		/* Add our pair to the hash table. Now, under the key index, we have the
		 * value string */
		thash_set(hash, key, value);
		dbg(1, "main(): Added pair (%s, %s)\n", key, value);
		i++;
	}

	/* TODO: Rest of examples */

	/* We're finished using memory from 'mm', so we can now deallocate all
	 * previously allocated memory using this manager */
	mmatic_free(mm);

	return 0;
}
