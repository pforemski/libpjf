/*
 * libasn_example - an exemplary application, demonstrating some basic concepts
 *
 * This file is part of libasn
 * Copyright (C) 2005-2010 ASN Sp. z o.o.
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
	/* This exemplary program does not have much sense, but please remember that
	 * its goal is to demonstrate how to use some of libasn library features */
	int i;
	char *pair, *ptr, *key, *value, *input;
	xstr *str;
	thash *hash;
	tlist *list;
	mmatic *mm;
	json *js;
	ut *unihash;

	if (argc < 3) {
		usage();
		return 0;
	}

	/* We fetch the debug level from the program arguments. The debug variable
	 * tells asn what dbg() messages should be viewed when encountered. Only
	 * those dbg() messages with equal or lower debug level then the one given in
	 * debug are handled */
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

	/* We allocate a dynamic buffer using the mm memory manager to use for input
	 * gathering */
	input = mmalloc(BUFSIZ);
	fprintf(stdout, "Please enter key of the element you wish to view or "
	                "^A D (EOF character) to continue:\n");
	while (fgets(input, BUFSIZ, stdin)) {
		input[strlen(input) - 1] = '\0';
		dbg(2, "main(): Inputted \"%s\"\n", input);
		/* We are fetching the value under the given user-typed key if existent */
		ptr = thash_get(hash, input);
		if (ptr) fprintf(stdout, "Value: %s\n", ptr);
		else     fprintf(stdout, "Key not found\n");
	}

	/* If, for some reason, we do not want to wait with freeing memory, we can
	 * free mmallocated memory using the mmfreeptr() function */
	mmfreeptr(input);

	xstr_set(str, "");
	thash_reset(hash);
	while ((ptr = (char *)thash_iter(hash, &key))) {
		xstr_append(str, ptr);
		xstr_append_char(str, '\n');
	}

	/* We want to pass our data to some external program. Here, we want to obtain
	 * the list of keys once more sorted according to the indexes. Since thash
	 * elements are now probably scatered differently than in the list, we would
	 * like to use the sort command. For this, we'll use asn_cmd(). */

	/* The used xstr_string() and xstr_length() macros have been defined for use
	 * to make the code more readable if required */
	asn_cmd("sort", NULL, NULL, xstr_string(str), xstr_length(str),
	        xstr_string(str), xstr_length(str), NULL, 0);

	fprintf(stdout, "Once again sorted values:\n%s\n", xstr_string(str));

	/* Now, let's use some unitype and json magic to turn our earlier hash table
	 * into a string in the JSON format */

	js = json_create(mm);
	/* What we need to do is create an unitype object from our earlier hash. The
	 * unitype library provides an abstraction layer for multi-typed objects.
	 * The libasn json library works on unitype - and a valid ut object is all we
	 * need to generate the JSON string */
	unihash = ut_new_thash(hash, mm);
	fprintf(stdout, "Now as a JSON string:\n%s\n", json_print(js, unihash));

	/* We're finished using memory from 'mm', so we can now deallocate all
	 * previously allocated memory using this manager */
	mmatic_free(mm);

	return 0;
}
