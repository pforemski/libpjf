/*
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

#ifndef _FCML_H_
#define _FCML_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/** Main FCML structure */
typedef struct fcmlparser_ {
	/** Caches parser output (of fcmlfile type) */
	thash *files;

	/** Filespaces (of fcmlfspace type) */
	thash *spaces;

	/** mmatic */
	mmatic *mm;
} fcmlparser;

/** Source to use instead of file->fp */
typedef struct _fcmlsrc_ {
	/** Description to help the user identify the source of error
	 * @param if empty, thats the final source (usually the FCML file contents) */
	char *descr;

	/** Current column */
	int col;

	/** Current line */
	int line;

	/** The data to read */
	char *data;

	/** Current byte number */
	int i;

	/** Next source to use if this ends */
	struct _fcmlsrc_ *next;
} _fcmlsrc;

/** Represents a single FCML file along with parser parameters */
typedef struct fcmlfile_ {
	/** Way back */
	fcmlparser *parser;

	/** Path to file */
	char path[PATH_MAX];

	/** Variables in root (of fcmlvar type) */
	thash *vars;

	/* File content sources */
	_fcmlsrc *addsrc;
} fcmlfile;

/** Represents a filespace definition */
typedef struct fcmlfspace_ {
	/** Root dir */
	char rootdir[PATH_MAX];
} fcmlfspace;

/** Type of an FCML variable */
enum fcmltype {
	STRING,
	ARRAY,
	NUMBER
};

/** An FCML variable */
typedef struct fcmlvar_ {
	enum fcmltype type;
	union in_t {
		char *string;
		thash *array;
		int num;
	} in;
} fcmlvar;

/*****/
void fcml_free_parser(void *arg);
void fcml_free_file(void *arg);
void fcml_free_var(void *arg);
void fcml_init_parser(fcmlparser *parser, mmatic *mm);
void fcml_init_file(fcmlfile *file, char *path, fcmlparser *parser);
void fcml_init_string(fcmlvar *var);
void fcml_init_array(fcmlvar *var, fcmlparser *parser);

void fcml_read_key(fcmlfile *file, char *keyname);
void fcml_parse_string(fcmlfile *file, fcmlvar *var);
void fcml_parse_eof(fcmlfile *file, fcmlvar *var);
void fcml_parse_include(fcmlfile *file, fcmlvar *var);
void fcml_parse_exec(fcmlfile *file, fcmlvar *var);
void fcml_parse_array(fcmlfile *file, fcmlvar *var);
void fcml_parse_value(fcmlfile *file, fcmlvar *var);
fcmlfile *fcml_parse(fcmlparser *parser, char *path, int nocache);

thash *fcml_get_vars(fcmlparser *parser, const char *path);
char *fcml_get_string(thash *vars, const char *name);
thash *fcml_get_array(thash *vars, const char *name);
int fcml_get_int(thash *vars, const char *name);
void fcml_set_string(thash *vars, const char *name, char *value, fcmlfile *file);

#endif
