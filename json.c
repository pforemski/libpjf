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

	return ut_new_tlist(list, json->mm);
}

static ut *parse_object(json *json)
{
	char c;
	thash *hash = thash_create(NULL, NULL, NULL, true, json->mm);
	ut *key, *val;

	c = SKIPWS();
	if (c != '{')
		return err(json, 2, "object: expected '{'");

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

	return ut_new_thash(hash, json->mm);
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

	return j;
}
