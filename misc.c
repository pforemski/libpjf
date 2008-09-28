/*
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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <syslog.h>

#include "misc.h"
#include "tlist.h"
#include "mmatic.h"
#include "xstr.h"

#ifndef NODEBUG
void dbg(int level, char *dbg, ...)
{
	int i;
	va_list args;
	static char buf[BUFSIZ];

	if (level > debug || level >= sizeof(buf)) return;

	va_start(args, dbg);
	if ((void (*)()) debugcb == (void (*)()) syslog ||
	    (void (*)()) debugcb == (void (*)()) vsyslog) {
		vsyslog(LOG_INFO, dbg, args);
		return;
	}
	else {
		for (i = 0; i < level; i++) buf[i] = ' ';
		if (vsnprintf(buf+level, sizeof(buf)-level-1, dbg, args))
			buf[sizeof(buf)-1] = '\0';
	}
	va_end(args);

	if (debugcb)
		(*debugcb)(buf);
	else
		fputs(buf, stderr);
}
#endif

void _die(const char *file, unsigned int line, char *msg, ...)
{
	va_list args;

	fprintf(stderr, "%s:%u: ", file, line);
	if (msg) {
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
	}
	else {
		fprintf(stderr, "aborting\n");
	}

	abort();
}

int asn_isfile(const char *path)
{
	struct stat stats;

	if (stat(path, &stats)) return -1;
	if (S_ISREG(stats.st_mode)) return 1;
	if (S_ISLNK(stats.st_mode)) return 2;
	return -2;
}

int asn_isfifo(const char *path)
{
	struct stat stats;

	if (stat(path, &stats)) return -1;
	if (S_ISFIFO(stats.st_mode)) return 1;
	return -2;
}

void asn_cd(const char *path)
{
	dbg(11, "changedir(%s)\n", path);

	if (chdir(path)) {
		dbg(3, "chdir(%s): %s\n", path, strerror(errno));
		die("Could not chdir() into %s\n", path);
	}
}

void asn_parsepath(const char *path, tlist *lpath, mmatic *mm)
{
	char *part, *ptr;

	path = mmstrdup(path);

	/* parse path into tlist */
	while ((part = strtok_r((char *) path, "/", &ptr))) {
		/* we won't need it anymore */
		path = NULL; /* XXX: required by strtok_r */

		switch (part[0]) {
			case '\0': /* ^/, // */
				break;
			case '.':
				/* check if ain't "./" nor "../" */
				switch(part[1]) {
					case '\0': /* ./ -- ignore */
						continue;
					case '.':  /* .. */
						if (part[2] != '\0')
							break; /* ..? -- go further */
						else
							tlist_pop(lpath); /* ../ -- go level down */
						continue;
					default: /* .? -- go further */
						break;
				}
				/* fall through */
			default: /* not "to ignore" nor "../" -- add */
				tlist_push(lpath, part);
				break;
		}
	}
}

char *asn_parsedoubleslashes(const char *vcwd, const char *vpath, mmatic *mm)
{
	tlist *list;

	/* get new path in form of a list */
	list = MMTLIST_CREATE(NULL);
	if (vpath[0] != '/') asn_parsepath(vcwd, list, mm);
	asn_parsepath(vpath, list, mm);

	return asn_makepath(list, mm);
}

char *asn_makepath(tlist *pathparts, mmatic *mm)
{
	char *part;
	xstr *ret = xstr_create("", mm);

	tlist_reset(pathparts);
	while ((part = tlist_iter(pathparts))) {
		xstr_append_char(ret, '/');
		xstr_append(ret, part);
	}

	/* handle "/" border case */
	if (ret->s[0] == 0) xstr_set(ret, "/");

	return ret->s;
}

int asn_isdir(const char *path)
{
	struct stat dirstat;

	if (stat(path, &dirstat)) return -1;
	if (!S_ISDIR(dirstat.st_mode)) return -2;
	return 1;
}

char *asn_abspath(const char *path, mmatic *mm)
{
	char cwd[PATH_MAX];

	if (path[0] == '/')
		return mmstrdup(path);
	else
		return mmprintf("%s/%s", getcwd(cwd, sizeof(cwd)), path);
}

int asn_mkdir(const char *path, mmatic *mm, int (*filter)(const char *part))
{
	tlist *list = MMTLIST_CREATE(NULL);
	char *curdir, *part;

	path = asn_abspath(path, mm);
	asn_parsepath(path, list, mm);

	curdir = mmalloc(strlen(path) + 2);
	curdir[0] = 0;

	tlist_reset(list);
	while ((part = tlist_iter(list))) {
		strcat(curdir, "/");
		strcat(curdir, part);

		switch (asn_isdir(curdir)) {
			case  1: continue;  /* exists, is a directory */
			case -2: return 0;  /* exists, is NOT a directory */
		}

		/* does not exist */
		if (filter && (*filter)(part) == 1) return 1;

		/* stat() error - try to mkdir() */
		if (errno != ENOENT) return 0; /* unknown error */
		if (mkdir(curdir, 0755) != 0) return 0; /* mkdir() failed */
	}

	return 1;
}

int asn_rmdir(const char *path, const char *skip)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *subd;
	char *newpath;
	int upret;

	if (lstat(path, &statbuf) < 0) return 0;

	if (S_ISDIR(statbuf.st_mode)) {
		subd = opendir(path);
		if (!subd) return 0;

		while ((dirp = readdir(subd))) {
			if (streq(dirp->d_name, ".")) continue;
			if (streq(dirp->d_name, "..")) continue;
			if (skip && streq(dirp->d_name, skip)) continue;

			newpath = asn_malloc(strlen(path) + strlen(dirp->d_name) + 2);
			strcpy(newpath, path);
			strcat(newpath, "/");
			strcat(newpath, dirp->d_name);
			upret = asn_rmdir(newpath, skip);
			free(newpath);
			if (!upret) return 0;
		}
		closedir(subd);

		if (rmdir(path) != 0 && !skip) return 0;
	}
	else {
		if (unlink(path) != 0) return 0;
	}

	return 1;
}

int isnumber(const char *str)
{
	int i;

	for (i = 0; str[i]; i++)
		if (!isdigit(str[i]))
			return 0;

	return 1;
}

char *asn_sanepath(const char *path, mmatic *mm)
{
	int i, j;
	char *ret, prev = 0;

	dbg(10, "sanepath(%s)\n", path);
	ret = mmalloc(strlen(path) + 1);

	for (i = j = 0; path[i]; i++) {
		if (path[i] == '/' && prev == '/') continue;
		prev = ret[j++] = path[i];
	}

	dbg(12, "sanepath(): j=%d\n", j);
	ret[j - (j > 1 && ret[j-1] == '/')] = 0;

	dbg(11, "sanepath(): %s\n", ret);
	return ret;
}

char *asn_readfile(const char *path, mmatic *mm)
{
	FILE *fp;
	char *buf, *rbuf;
	unsigned int bigbufsiz, bufsiz;

	if ((fp = fopen(path, "r")) == NULL)
		return NULL;

	bigbufsiz = bufsiz = 4096;
	rbuf = buf = mmalloc(bufsiz);

	for (;;) {
		memset(buf, 0, bufsiz);
		fread(buf, 1, bufsiz, fp); /* fast? */

		if (feof(fp) || ferror(fp) || bigbufsiz > 100000000) break;

		bufsiz = bigbufsiz;
		bigbufsiz *= 2;

		rbuf = mmrealloc(bigbufsiz, rbuf);
		buf = rbuf + bufsiz; /* start from half */
	}

	fclose(fp);
	return rbuf;
}

int asn_writefile(const char *path, const char *s)
{
	FILE *fp;
	int r;

	if ((fp = fopen(path, "w")) == NULL) {
		dbg(3, "fopen(%s): %s\n", path, strerror(errno));
		return -1;
	}

	r = fputs(s, fp);
	fclose(fp);
	return r;
}

unsigned int asn_utimestamp()
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0)
		return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
	else
		return 0;
}
