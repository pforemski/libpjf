/*
 * This file is part of libasn
 * Copyright (C) 2008 Ivo van Poorten
 * Copyright (C) 2009 ASN Sp. z o.o.
 *
 * License: GNU Lesser General Public License version 2.1
 * Imported from zzjson library by: Pawel Foremski <pjf@asn.pl>
 */

#include <ctype.h>
#include <string.h>
#include <math.h>

#include "lib.h"

#define GETC()          (json->txt[json->i++])
#define UNGETC()        (json->i--)
#define SKIPWS()        skipws(json)

#define INC_DEPTH() do {  \
	json->depth++;        \
	if (json->depth > 50) \
		return err(json, 20, "document too deep"); \
} while (0);

#define DEC_DEPTH() do {  \
	json->depth--;        \
	if (json->depth < 0)  \
		return err(json, 21, "internal error"); \
} while (0);

static ut *parse_array(json *json);
static ut *parse_object(json *json);

static char skipws(json *json)
{
	char c;

	while ((c = GETC())) {
		switch (c) {
			case ' ':
			case '\f':
			case '\n':
			case '\r':
			case '\t':
			case '\v':
				continue;
		}
		break;
	}

	return c;
}

static ut *err(json *json, int code, const char *msg)
{
	return ut_new_err(
		code,
		msg,
		mmatic_printf(json->mm, "i=%d", json->i), // TODO
		json->mm);
}

static ut *parse_string(json *json)
{
	char c;
	xstr *str = xstr_create("", json->mm);

	c = SKIPWS();
	if (c != '"')
		return err(json, 8, "string: expected \" at the start");

	while ((c = GETC()) && c != '"') {
		if (c >= 0 && c <= 31) {
			return err(json, 9, "string: control characters not allowed");
		}
		else if (c == '\\') {
			c = GETC();
			switch (c) {
				case 'b': c = '\b'; break;
				case 'f': c = '\f'; break;
				case 'n': c = '\n'; break;
				case 'r': c = '\r'; break;
				case 't': c = '\t'; break;
				case 'u':
					UNGETC();    /* ignore \uHHHH, copy verbatim */
					c = '\\';
					break;
				case '\\':
				case '/':
				case '"':
					break;
				default:
					return err(json, 10, "string: illegal escape character");
			}
		}

		xstr_append_char(str, c);
	}

	if (c != '"')
		return err(json, 11, "string: expected \" at the end");

	return ut_new_xstr(str, json->mm);
}

static ut *parse_number(json *json)
{
	unsigned long long ival = 0, expo = 0;
	double frac = 0.0, fracshft = 10.0;
	int sign = 1, signexpo = 1;
	char c;
	bool dbl = 0;

	c = SKIPWS();
	if (c == '-') { sign = -1; c = GETC(); }
	if (c == '0') { c = GETC(); goto skip; }

	if (!isdigit(c))
		return err(json, 16, "number: digit expected");

	while (isdigit(c)) {
		ival *= 10;
		ival += c - '0';
		c = GETC();
	}

skip:
	if (c != '.') goto skipfrac;
	else dbl = true;

	c = GETC();
	if (!isdigit(c))
		return err(json, 17, "number: digit expected");

	while (isdigit(c)) {
		frac += (double)(c - '0') / fracshft;
		fracshft *= 10.0;
		c = GETC();
	}

skipfrac:
	if (c != 'e' && c != 'E') goto skipexpo;
	else dbl = true;

	c = GETC();
	if (c == '+') c = GETC();
	else if (c == '-') { signexpo = -1; c = GETC(); }

	if (!isdigit(c))
		return err(json, 18, "number: digit expected");

	while (isdigit(c)) {
		expo *= 10;
		expo += c - '0';
		c = GETC();
	}

skipexpo:
	UNGETC();

	if (dbl)
		return ut_new_double(
			(sign * ((long long) ival + frac)) * pow(10.0, (double) signexpo * expo),
			json->mm);
	else
		return ut_new_int(
			sign * ival,
			json->mm);
}

static bool check_literal(json *json, const char *s)
{
	int i = json->i, l = strlen(s);

	json->i += l;
	return (strncmp(json->txt + i, s, strlen(s)) == 0);
}

static ut *parse_true(json *json)
{
	if (check_literal(json, "true"))
		return ut_new_bool(true, json->mm);
	else
		return err(json, 12, "true: expected \"true\"");
}

static ut *parse_false(json *json)
{
	if (check_literal(json, "false"))
		return ut_new_bool(false, json->mm);
	else
		return err(json, 13, "false: expected \"false\"");
}

static ut *parse_null(json *json)
{
	if (check_literal(json, "null"))
		return ut_new_null(json->mm);
	else
		return err(json, 14, "null: expected \"null\"");
}

static ut *parse_value(json *json)
{
	char c;

	c = SKIPWS(); UNGETC();
	switch (c) {
		case '"': return parse_string(json);
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '-':
		          return parse_number(json);
		case '{': return parse_object(json);
		case '[': return parse_array(json);
		case 't': return parse_true(json);
		case 'f': return parse_false(json);
		case 'n': return parse_null(json);
	}

	return err(json, 15, "value: encountered invalid character");
}

static ut *parse_array(json *json)
{
	tlist *list = tlist_create(NULL, json->mm);
	ut *val;
	char c;

	c = SKIPWS();
	if (c != '[')
		return err(json, 3, "array: expected '['");

	INC_DEPTH();

	c = SKIPWS();
	while (c > 0 && c != ']') {
		UNGETC();

		val = parse_value(json);
		if (!ut_ok(val)) // TODO: convert into exceptions?
			return val;

		c = SKIPWS();
		if (!(c == ',' || c == ']'))
			return err(json, 4, "array: expected ',' or ']'");

		tlist_push(list, val);

		if (c == ',')
			c = SKIPWS();
	}

	if (c != ']')
		return err(json, 5, "array: expected ']'");

	DEC_DEPTH();
	return ut_new_uttlist(list, json->mm);
}

static ut *parse_object(json *json)
{
	char c;
	thash *hash = thash_create(NULL, NULL, NULL, true, json->mm);
	ut *key, *val;

	c = SKIPWS();
	if (c != '{')
		return err(json, 2, "object: expected '{'");

	INC_DEPTH();

	c = SKIPWS();
	while (c > 0 && c != '}') {
		UNGETC();

		key = parse_string(json);
		if (!ut_ok(key)) return key;

		c = SKIPWS();
		if (c != ':')
			return err(json, 6, "object: expected ':'");

		val = parse_value(json);
		if (!ut_ok(val)) return val;

		c = SKIPWS();
		if (!(c == ',' || c == '}'))
			return err(json, 7, "object: expected ',' or '}'");

		thash_set(hash, ut_char(key), val);

		if (c == ',')
			c = SKIPWS();
	}

	if (c != '}')
		return err(json, 19, "object: expected '}'");

	DEC_DEPTH();
	return ut_new_utthash(hash, json->mm);
}

ut *json_parse(json *json, const char *txt)
{
	json->txt = txt;
	json->i = 0;
	return parse_value(json);
}

json *json_create(mmatic *mm)
{
	json *j = mmalloc(sizeof(json));

	j->mm = mm;
	j->txt = NULL;
	j->i = 0;
	j->depth = 0;

	return j;
}

char *json_escape(json *json, const char *str)
{
	bool bs;
	char c;
	xstr *xs = xstr_create("", json->mm);

	xstr_reserve(xs, 1.1 * strlen(str));

	while ((c = *str++)) {
		bs = true;
		switch (c) {
			case '\\':
				if (*str == 'u') bs = false;  /* copy \uHHHH verbatim */
				break;
			case '"':               break;
			case '\b':  c = 'b';    break;
			case '\f':  c = 'f';    break;
			case '\n':  c = 'n';    break;
			case '\r':  c = 'r';    break;
			case '\t':  c = 't';    break;
			default:    bs = false; break;
		}

		if (bs) xstr_append_char(xs, '\\');
		xstr_append_char(xs, c);
	}

	return xstr_string(xs);
}

char *json_print(json *json, ut *var)
{
	mmatic *mm = json->mm;
	char *k, *str;
	ut *el;
	xstr *xs;

	switch (var->type) {
		case T_STRING:
			str = mmprintf("\"%s\"",
				json_escape(json, xstr_string(var->d.as_xstr)));
			break;

		case T_INT:
			str = mmprintf("%d", var->d.as_int);
			break;

		case T_DOUBLE:
			str = mmprintf("%g", var->d.as_double);
			break;

		case T_LIST:
			xs = MMXSTR_CREATE("[ ");

			TLIST_ITER_LOOP(var->d.as_tlist, el) {
				xstr_append(xs, json_print(json, el));
				xstr_append(xs, ", ");
			}
			if (xstr_length(xs) > 2) xstr_cut(xs, 2);
			xstr_append(xs, " ]");

			str = xstr_string(xs);
			break;

		case T_HASH:
			xs = MMXSTR_CREATE("{ ");

			THASH_ITER_LOOP(var->d.as_thash, k, el) {
				xstr_append_char(xs, '"');
				xstr_append(xs, k);
				xstr_append(xs, "\": ");

				xstr_append(xs, json_print(json, el));
				xstr_append(xs, ", ");
			}
			if (xstr_length(xs) > 2) xstr_cut(xs, 2);
			xstr_append(xs, " }");

			str = xstr_string(xs);
			break;

		case T_BOOL:
			str = var->d.as_bool ? "true" : "false";
			break;

		case T_NULL:
			str = "null";
			break;

		case T_ERR:
			if (var->d.as_err->data)
				str = mmprintf("{ \"code\": %d, \"message\": \"%s\", \"data\": \"%s\" }",
					var->d.as_err->code,
					json_escape(json, var->d.as_err->msg),
					json_escape(json, var->d.as_err->data));
			else
				str = mmprintf("{ \"code\": %d, \"message\": \"%s\" }",
					var->d.as_err->code,
					json_escape(json, var->d.as_err->msg));
			break;

		default:
			str = "";
			break;
	}

	return str;
}