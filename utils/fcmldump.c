/*
 * This file is part of libasn
 * Copyright (C) 2005-2010 ASN Sp. z o.o.
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

/*
 *
 * !!! NOTE WARNING NOTE WARNING NOTE WARNING NOTE WARNING NOTE WARNING NOTE !!!
 * !!!                                                                       !!!
 * !!!      THIS PROGRAM IS INSECURE. USE IT ONLY IN SAFE ENVIRONMENTS.      !!!
 * !!!                                                                       !!!
 * !!! NOTE WARNING NOTE WARNING NOTE WARNING NOTE WARNING NOTE WARNING NOTE !!!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"

char indent[100];
int il = 0;
int quote = 1;

int debug = 2;
void (*debugcb)() = NULL;

void infill()
{
	int i;
	for (i = 0; i < il; i++) indent[i] = ' ';
	indent[i] = 0;
}

void ilup() { il += 4; infill(); }
void ildown() { il -= 4; infill(); }

void print_var(fcmlvar *var);

void print_string(fcmlvar *var)
{
	if (quote)
		printf("\"%s\"", var->in.string);
	else
		printf("%s", var->in.string);
}

void print_array(fcmlvar *var)
{
	char *key;
	fcmlvar *el;

	printf("{\n");
	ilup();
	thash_reset(var->in.array);
	while ((el = thash_iter(var->in.array, &key))) {
		printf("%s%s: ", indent, key);
		print_var(el);
		printf(",\n");
	}
	ildown();
	printf("%s}", indent);

}

void print_var(fcmlvar *var)
{
	switch (var->type) {
		case STRING: print_string(var); break;
		case ARRAY: print_array(var); break;
		case NUMBER: printf("%d", var->in.num); break;
	}
}

int main(int argc, char *argv[])
{
	fcmlfile *file;
	fcmlvar *var = 0;
	fcmlfspace *sp;
	fcmlparser fp;
	int i;
	char *key;
	thash *vars;

	mmatic *mm = mmatic_create();

	if (argc < 2) {
		fprintf(stderr, "Usage: fcmldump [--debug=<level>] [--<filespace>=root] <file.fc> [var1 [var2 ...]]\n");
		return 1;
	}

	fcml_init_parser(&fp, mm);

	argv++;
	while (*argv && argv[0][0] == '-' && argv[0][1] == '-') {
		if (!strncmp(*argv, "--debug=", 8) && strlen(*argv) > 8) {
			debug = atoi(*argv + 8);
		}
		else {
			*argv += 2;
			for (i = 1; argv[0][i] && argv[0][i] != '='; i++);

			if (argv[0][i] != '=') return 1;
			else argv[0][i] = 0;

			sp = mmalloc(sizeof(fcmlfspace));
			thash_set(fp.spaces, *argv, sp);

			*argv += i+1;
			strcpy(sp->rootdir, *argv);
		}

		argv++;
		argc--;
	}

	if (!(file = fcml_parse(&fp, *argv, 1)))
		return 1;

	if (argc == 2) while ((var = thash_iter(file->vars, &key))) {
		printf("%s: ", key);
		print_var(var);
		printf("\n");
	}
	else {
		vars = file->vars;
		for (i = 1; i < argc-1; i++) {
			if (var) {
				if (var->type != ARRAY) {
					fprintf(stderr, "Variable not found (#1)\n");
					return 1;
				}
				vars = var->in.array;
			}

			var = thash_get(vars, argv[i]);
			if (!var) {
				fprintf(stderr, "Variable not found (#2)\n");
				return 1;
			}
		}

		quote = 0;
		print_var(var);
		printf("\n");
	}

	mmsummary(9);
	fcml_free_parser(&fp);
	mmatic_free(mm);

	return 0;
}
