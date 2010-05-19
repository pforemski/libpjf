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
 *
 * TODO:
 * - threadsafety
 * - BUG: secure syntax parsing (not really a huge problem, except when
 *   variables are generated dynamically)
 *   - IDEA?: don't allow "!" and "@" in another "!"
 * - use tlist?
 * - use xstr?
 * - mmstr* where possible
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <setjmp.h>
#include <limits.h>
#include <unistd.h>

#include "lib.h"

/* helpers */
#define GET_ONE (c = get_one(file, __LINE__))
#define ONE_BACK one_back(file, __LINE__)
#define PEEK_ONE GET_ONE; ONE_BACK;

/** Stop on \n */
#define RUN_TILL_END while (GET_ONE != '\n');

/** Increments counters, ignores spaces, new lines and comments */
#define CONT_TRASH if (isspace(c)) continue; else if (c == '#') { RUN_TILL_END; continue; }

/** Skip whitespaces, comments, etc */
#define SKIP_TRASH while (GET_ONE) { CONT_TRASH; break; }

/** Maximal array entry key length */
#define KEYLEN 128

/** Shortcuts */
#define MM (file->parser->mm)
#define fmmalloc(size) (mmatic_alloc((size), file->parser->mm))
#define pmmalloc(size) (mmatic_alloc((size),       parser->mm))
#define fmmstrdup(str) (mmatic_strdup((str), file->parser->mm))

#define THASH_NEW_STR(ffn, mm) (thash_create(NULL, NULL, (ffn), 1, (mm)))
#define THASH_NEW_PTR(ffn, mm) (thash_create(thash_ptr_hash, NULL, (ffn), 0, (mm)))


/** For pseudo-exceptions */
static jmp_buf jb;

/** If 1, EOF is allowed */
static int leteof = 1;

/** Name of currently parsed variable */
static char *curvar;

/*****/
static void fcml_push_src(fcmlfile *file, char *descr, char *data)
{
	fcmlsrc *src = fmmalloc(sizeof(fcmlsrc)),
	         *first = file->addsrc;

	src->descr = descr;
	src->col = 1;
	src->line = 1;
	src->data = data;
	src->i = 0;
	src->next = first;
	file->addsrc = src;
}

static void fcml_pop_src(fcmlfile *file)
{
	fcmlsrc *src = file->addsrc;

	if (!src) return;

	file->addsrc = src->next;
	if (src->descr) mmfreeptr(src->descr);
	mmfreeptr(src->data);
	mmfreeptr(src);
}

static inline int get_one(fcmlfile *file, int line)
{
	int c;

read_byte:
	switch (c = (int) file->addsrc->data[file->addsrc->i++]) {
		case 0:
			if (file->addsrc && file->addsrc->descr) {
				fcml_pop_src(file); /* XXX: assumption after: file->addsrc != 0 */
				goto read_byte;
			}

			if (leteof) longjmp(jb, 666); /* finish! */

			dbg(1, "Unexpected end of file (CL%d)\n", line);
			longjmp(jb, 9);
		case '\n':
			file->addsrc->col = 1;
			file->addsrc->line++;
			break;
		default:
			file->addsrc->col++;
			break;
	}

	return c;
}

static inline int get_line(char *to, int size, fcmlfile *file, int line)
{
	int c, i;

	size--;
	for (i = 0; i < size && (c = get_one(file, line)) && c != '\n'; i++)
		to[i] = c;
	to[i] = 0;

	dbg(14, "get_line(): read '%s'\n", to);

	return i;
}

static inline void one_back(fcmlfile *file, int line)
{
	if (file->addsrc) {
		file->addsrc->i--;
		file->addsrc->col--; /* XXX: no check for <0 (assumption) */
	}
	else {
		dbg(1, "Unexpected end of file #2 (CL%d)\n", line);
		longjmp(jb, 9);
	}
}

/*****/
thash *fcml_get_vars(fcmlparser *parser, const char *path)
{
	static fcmlfile *file;
	file = (fcmlfile *) thash_get(parser->files, path);
	if (file) return file->vars;
	else return NULL;
}

char *fcml_get_string(thash *vars, const char *name)
{
	static fcmlvar *var;
	var = (fcmlvar *) thash_get(vars, name);
	if (var && var->type == STRING) return var->in.string;
	else return NULL;
}

thash *fcml_get_array(thash *vars, const char *name)
{
	static fcmlvar *var;
	var = (fcmlvar *) thash_get(vars, name);
	if (var && var->type == ARRAY) return var->in.array;
	else return NULL;
}

int fcml_get_int(thash *vars, const char *name)
{
	static fcmlvar *var;
	var = (fcmlvar *) thash_get(vars, name);
	if (var && var->type == NUMBER) return var->in.num;
	else return 0;
}

void fcml_set_string(thash *vars, const char *name, char *value, fcmlfile *file)
{
	static fcmlvar *var;

	var = fmmalloc(sizeof(fcmlvar));
	fcml_init_string(var);

	var->in.string = value;
	thash_set(vars, name, var);
}

/*****/
void fcml_free_parser(void *arg)
{
	fcmlparser *parser = (fcmlparser *) arg;
	if (parser->files) thash_free(parser->files);
	if (parser->spaces) thash_free(parser->spaces);
}

void fcml_free_file(void *arg)
{
	fcmlfile *file = (fcmlfile *) arg;
	fcmlsrc *src = file->addsrc, *src2;

	if (file->vars) thash_free(file->vars);

	while (src) {
		src2 = src->next;
		mmfreeptr(src->data);
		if (src->descr) mmfreeptr(src->descr);
		mmfreeptr(src);
		src = src2;
	}
}

void fcml_free_var(void *arg)
{
	fcmlvar *var = (fcmlvar *) arg;
	switch (var->type) {
		case STRING:
			if (var->in.string) {
				mmfreeptr(var->in.string);
				var->in.string = 0;
			}
			break;
		case ARRAY:
			if (var->in.array) {
				thash_free(var->in.array);
				var->in.array = 0;
			}
			break;
		case NUMBER: break;
	}
}

/*****/
void fcml_init_parser(fcmlparser *parser, mmatic *mm)
{
	parser->files = THASH_NEW_STR(fcml_free_file, mm);
	parser->spaces = THASH_NEW_STR(mmfreeptr, mm);
	parser->mm = mm;
}

void fcml_init_file(fcmlfile *file, char *path, fcmlparser *parser)
{
	strncpy(file->path, path, PATH_MAX);
	file->vars = THASH_NEW_STR(fcml_free_var, parser->mm);
	file->addsrc = NULL;
	file->parser = parser;
}

void fcml_init_string(fcmlvar *var)
{
	var->type = STRING;
	var->in.string = 0;
}

void fcml_init_array(fcmlvar *var, fcmlparser *parser)
{
	var->type = ARRAY;
	var->in.array = THASH_NEW_STR(fcml_free_var, parser->mm);
}

/*****/
void fcml_parse_string(fcmlfile *file, fcmlvar *var)
{
	char buf[8192];
	int i = 0, c, prevc = 0;

	fcml_init_string(var);

	/* wacky loop */
	while (GET_ONE) {
		switch (c) {
			case ' ':
			case '\t':
				if (prevc == '\n') continue;
				break;
			case '\n':
				prevc = '\n';
				continue;
			case '"':
				if (prevc == '\\') i--;
				else { buf[i] = 0; goto string_loop_end; }
				break;
		}

		buf[i++] = prevc = c;
		if (i == sizeof(buf)) {
			dbg(4, "fcml_parse_string(): string too long\n");
			longjmp(jb, 1);
		}
	}

string_loop_end:
	var->in.string = fmmalloc(i+1);
	strncpy(var->in.string, buf, i+1);
	dbg(10, "fcml_parse_string(): read string (len=%d): '%s'\n", i, var->in.string);
}

void fcml_parse_eof(fcmlfile *file, fcmlvar *var)
{
	char marker[100];
	char line[1024];
	char buf[8192];
	int i = 0, j = 0, c;

	fcml_init_string(var);
	while (GET_ONE) {
		if (isalnum(c)) {
			marker[i++] = c;
		}
		else {
			marker[i] = 0;
			if (c != '\n') RUN_TILL_END;
			break;
		}
	}

	dbg(10, "fcml_parse_eof(): eof marker: '%s'\n", marker);

	while (1) {
		i = get_line(line, sizeof(line), file, __LINE__);
		if (!strncmp(line, marker, sizeof(marker)))
			break;

		if (sizeof(buf)-j-1 <= i) {
			dbg(4, "fcml_parse_eof(): string too long\n");
			longjmp(jb, 2);
		}

		bcopy(line, buf+j, i);
		j += i;
		buf[j++] = '\n';
	}

	buf[j] = 0;
	dbg(10, "fcml_parse_eof(): buf: %s\n", buf);
	var->in.string = fmmstrdup(buf);
}

void fcml_parse_include(fcmlfile *file, fcmlvar *var)
{
	char space[KEYLEN];
	char path[PATH_MAX] = "";
	fcmlfspace *fs = 0;
	int c, i;
	fcmlvar pathvar;

	fcml_init_string(var);

	PEEK_ONE;

	/* has a filespace */
	if (isalnum(c)) {
		fcml_read_key(file, space);
		dbg(10, "fcml_parse_include(): has a filespace '%s'\n", space);

		fs = thash_get(file->parser->spaces, space);
		if (!fs) {
			dbg(1, "Filespace not defined: '%s'\n", space);
			longjmp(jb, 3);
		}
	}

	GET_ONE;
	if (c != '"') {
		dbg(4, "fcml_parse_include(): file name not enclosed in double braces\n");
		longjmp(jb, 4);
	}

	/* now read the filename */
	fcml_parse_string(file, &pathvar);

	/* append filespace path */
	if (fs) {
		strcpy(path, fs->rootdir);
		if (pathvar.in.string[0] != '/') strcat(path, "/");
	}
	strcat(path, pathvar.in.string);

	var->in.string = asn_readfile(path, MM);
	if (!var->in.string) {
		dbg(2, "fcml_parse_include(): fopen(%s): %s\n", path, strerror(errno));
		var->in.string = fmmstrdup("");
	}
	else {
		/* remove trailing \n */
		i = strlen(var->in.string);
		if (var->in.string[i-1] == '\n') var->in.string[i-1] = 0;
	}

	fcml_free_var(&pathvar);
}

void fcml_parse_exec(fcmlfile *file, fcmlvar *var)
{
	int c, rc, inlinescript = 0;
	char space[KEYLEN], cmdstr[PATH_MAX], buf[BUFSIZ], buf2[BUFSIZ];
	char *name;
	fcmlvar cmd;
	fcmlfspace *fs = 0;
	thash *env = THASH_NEW_STR(NULL, MM);

	fcml_init_string(var);

	switch (GET_ONE) {
		case '@': inlinescript = 1; fcml_parse_include(file, &cmd); break;
		case '>': inlinescript = 1; fcml_parse_eof(file, &cmd); break;
		case '"': fcml_parse_string(file, &cmd); break;
		default: /* exec with a filespace */
			/* read filespace */
			ONE_BACK;
			fcml_read_key(file, space);
			dbg(10, "has a filespace '%s'\n", space);

			/* get filespace definition */
			fs = thash_get(file->parser->spaces, space);
			if (!fs) { dbg(1, "Filespace not defined: '%s'\n", space); longjmp(jb, 10); }

			/* position on command */
			if (GET_ONE != '"') { dbg(4, "command name not enclosed in double braces\n"); longjmp(jb, 11); }

			/* now read the command */
			fcml_parse_string(file, &cmd);

			/* append filespace path */
			if (fs) strcpy(cmdstr, fs->rootdir);
			strcat(cmdstr, cmd.in.string);

			/* copy full path to cmd */
			mmfreeptr(cmd.in.string);
			cmd.in.string = fmmstrdup(cmdstr);
			break;
	}

	/* export filespace paths in FCML_* */
	thash_reset(file->parser->spaces);
	while ((fs = thash_iter(file->parser->spaces, &name))) {
		snprintf(buf, sizeof(buf), "FCML_%s", name);
		thash_set(env, buf, fs->rootdir);
	}

	/* run */
	xstr *output = xstr_create("", MM);
	rc = asn_cmd2(cmd.in.string, NULL, env, NULL, output, NULL);
	if (rc != 0 || xstr_length(output) == 0) {
		dbg(2, "child failed\n");
		var->in.string = fmmstrdup("");
		goto exec_free;
	}

	/* make a description */
	if (inlinescript)
		snprintf(buf2, sizeof(buf2), "inline script");
	else
		snprintf(buf2, sizeof(buf2), "external command '%s'", cmd.in.string);

	dbg(9, "child succeeded, parsing\n");
	fcml_push_src(file, fmmstrdup(buf2), xstr_string(output));
	fcml_parse_value(file, var);

exec_free:
	thash_free(env);
	fcml_free_var(&cmd);
	return;
}

void fcml_read_key(fcmlfile *file, char *keyname)
{
	int c, i = 0;

	dbg(11, "fcml_read_key(): reading the key\n");

	while (GET_ONE) {
		dbg(15, "fcml_read_key(): read '%c'\n", c);
		if (isalnum(c)) {    /* reading the key */
			keyname[i++] = c;
			if (i == KEYLEN) longjmp(jb, 6);
		}
		else if (c == ':') { /* end of key */
			keyname[i] = 0;
			break;
		}
		else {
			dbg(4, "fcml_read_key(): couldn't read the key\n");
			longjmp(jb, 7);
		}
	}
}

void fcml_parse_array(fcmlfile *file, fcmlvar *var)
{
	int c;
	char keyname[KEYLEN];
	fcmlvar *el;
	fcmlvar *counter;

	fcml_init_array(var, file->parser);

parse_element:
	keyname[0] = 0;

	SKIP_TRASH;
	ONE_BACK; /* position on val/key beginning */

	/* has key */
	if (isalnum(c))
		fcml_read_key(file, keyname);

	/* now we should be "on" value */

	/* create array element, add it */
	el = fmmalloc(sizeof(fcmlvar));

	/* named key */
	if (keyname[0]) {
		dbg(10, "fcml_parse_array(): adding key '%s'\n", keyname);
		thash_set(var->in.array, keyname, el);
	}

	/* numeric key - get/create counter, increment, save */
	counter = (fcmlvar *) thash_get(var->in.array, ":counter");
	if (counter == NULL) {
		counter = fmmalloc(sizeof(fcmlvar));
		counter->type = NUMBER;
		counter->in.num = 1;
		thash_set(var->in.array, ":counter", counter);
	}
	else {
		counter->in.num++;
		dbg(10, "fcml_parse_array(): array counter: %d\n", counter->in.num);
	}
	snprintf(keyname, sizeof(keyname), ":%d", counter->in.num);
	thash_set(var->in.array, keyname, el);

	/* should land after the value */
	fcml_parse_value(file, el);

	SKIP_TRASH;

	switch (c) {
		case EOF:
		case '}': return;
		case ',': break;
		default: ONE_BACK;
	}

	goto parse_element;
}

void fcml_parse_value(fcmlfile *file, fcmlvar *var)
{
	int c;

	SKIP_TRASH;

	dbg(10, "fcml_parse_value(): first value character: '%c' - parsing as ", c);
	switch (c) {
		case '"':
			dbg(10, "string\n");
			fcml_parse_string(file, var);
			return;
		case '>':
			dbg(10, "till-eof\n");
			fcml_parse_eof(file, var);
			return;
		case '@':
			dbg(10, "include-file\n");
			fcml_parse_include(file, var);
			return;
		case '!':
			dbg(10, "exec\n");
			fcml_parse_exec(file, var);
			return;
		case '{':
			dbg(10, "array\n");
			fcml_parse_array(file, var);
			return;
		default:
			dbg(10, "UNKNOWN!\n");
			longjmp(jb, 8);
	}
}

fcmlfile *fcml_parse(fcmlparser *parser, char *path, int nocache)
{
	fcmlfile *file;
	fcmlvar *var;
	int c, i, rc;
	char varname[100] = "", filepath[PATH_MAX], *buf;

	/* get full path */
	if (realpath(path, filepath) == NULL) {
		dbg(4, "fcml_parse(): realpath(%s): %s\n", path, strerror(errno));
		return NULL;
	}

	dbg(4, "fcml_parse(): parsing %s\n", filepath);

	file = thash_get(parser->files, filepath);
	if (file) {
		if (nocache) {
			dbg(6, "fcml_parse(): deleting file from cache\n");
			thash_set(parser->files, filepath, NULL);
		}
		else {
			dbg(6, "fcml_parse(): returning file from cache\n");
			return file;
		}
	}
	else {
		dbg(6, "fcml_parse(): file not found in cache - parsing\n");
	}

	file = pmmalloc(sizeof(fcmlfile));
	thash_set(parser->files, filepath, (void *) file);

	fcml_init_file(file, filepath, parser);

	dbg(10, "fcml_parse(): reading whole file\n");
	buf = asn_readfile(file->path, MM);
	if (!buf) return NULL;
	fcml_push_src(file, 0, buf);

	if ((rc = setjmp(jb)) != 0)
		goto parse_error;

	while (GET_ONE) {
		/* skip comments, etc. */
		CONT_TRASH;

		dbg(10, "fcml_parse(): seeking new variable name\n");
		memset(varname, 0, sizeof(varname));

		varname[i=0] = c;
		leteof = 0;

		/* search for variable name */
		while (GET_ONE) {
			if (isalnum(c)) {
				varname[++i] = c;
				if (i == sizeof(varname)) return NULL;
			}
			else {
				varname[++i] = 0;
				break;
			}
		}

		/* TODO: support the '+=' operator */
		while (c != '=') GET_ONE;

		dbg(9, "fcml_parse(): PARSING VARIABLE '%s'\n", varname);
		curvar = varname;

		/* read the value after variable name */
		var = pmmalloc(sizeof(fcmlvar));
		thash_set(file->vars, varname, var);

		if ((rc = setjmp(jb)) == 0) {
			fcml_parse_value(file, var);
		}
		else {
parse_error:
			if (rc == 666) goto file_end;

			dbg(0, "%s/%s: parse error #%d at %d:%d", path, curvar, rc, file->addsrc->line, file->addsrc->col);
			while (file->addsrc && file->addsrc->descr) {
				dbg(0, " of %s", file->addsrc->descr);
				fcml_pop_src(file);
				dbg(0, " at %d:%d", file->addsrc->line, file->addsrc->col);
			}
			dbg(0, " of FCML file\n");

			return NULL;
		}

		leteof = 1;
	}

file_end:
	/* TODO: support filepath.d/ extensions */

	return file;
}

/*
 * vim: tw=120
 */
