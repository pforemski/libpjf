/*
 * This file is part of libpjf
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Author: Pawel Foremski <pawel@foremski.pl>
 *
 * libpjf is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * libpjf is distributed in the hope that it will be useful, but WITHOUT ANY
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

#include "lib.h"

#ifndef NODEBUG
void _dbg(const char *file, unsigned int line, const char *fn, int level, char *fmt, ...)
{
	int i;
	va_list args;
	static char buf[BUFSIZ];
	struct timeval tv;

	if (level > debug || level >= sizeof(buf)) return;

	gettimeofday(&tv, NULL);

	va_start(args, fmt);
	if ((void (*)()) debugcb == (void (*)()) syslog ||
	    (void (*)()) debugcb == (void (*)()) vsyslog) {
		/* TODO: prepend fn */
		vsyslog(LOG_INFO, fmt, args);
		return;
	}

	for (i = 0; i < level && i < sizeof(buf); i++) buf[i] = ' ';

	if (debug >= 10)
		snprintf(buf, sizeof(buf), "[%6u.%06u] %s:%u %s(): ",
			(uint32_t) tv.tv_sec, (uint32_t) tv.tv_usec, file, line, fn);
	else
		snprintf(buf, sizeof(buf), "[%6u.%06u] %s(): ",
			(uint32_t) tv.tv_sec, (uint32_t) tv.tv_usec, fn);

	i = strlen(buf);
	vsnprintf(buf+i, sizeof(buf)-i-1, fmt, args);
	buf[sizeof(buf)-1] = '\0';

	va_end(args);

	if (debugcb)
		(*debugcb)(buf);
	else
		fputs(buf, stderr);
}
#endif

void _die(const char *file, unsigned int line, const char *fn, char *msg, ...)
{
	va_list args;

	fprintf(stderr, "%s:%u %s(): ", file, line, fn);
	if (msg) {
		va_start(args, msg);
		vfprintf(stderr, msg, args);
		va_end(args);
	} else {
		fprintf(stderr, "aborting");
	}
	fprintf(stderr, "\n");

	abort();
}

void *pjf_malloc(size_t size)
{
	void *mem;

	dbg(15, "%u\n", size);

	mem = malloc(size);
	if (!mem)
		die("pjf_malloc(%u) failed, out of memory\n", (unsigned int) size);

	return mem;
}

/* it seems its easier to have code duplication than coping with va_lists */
char *pjf_malloc_printf(const char *fmt, ...)
{
	va_list args; char *buf;
	va_start(args, fmt); buf = pjf_malloc(BUFSIZ); vsnprintf(buf, BUFSIZ, fmt, args); va_end(args);
	return buf;
}

int pjf_isfile(const char *path)
{
	struct stat stats;

	if (stat(path, &stats)) return -1;
	if (S_ISREG(stats.st_mode)) return 1;
	if (S_ISLNK(stats.st_mode)) return 2;
	return -2;
}

bool pjf_isexecutable(const char *path)
{
	struct stat stats;

	if (stat(path, &stats)) return false;

	/* XXX: simplified check, ie. we dont know if users or groups rights apply */
	return (
		(S_ISREG(stats.st_mode) || S_ISLNK(stats.st_mode)) &&
		(stats.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)));
}

int pjf_isfifo(const char *path)
{
	struct stat stats;

	if (stat(path, &stats)) return -1;
	if (S_ISFIFO(stats.st_mode)) return 1;
	return -2;
}

void pjf_cd(const char *path)
{
	dbg(11, "changedir(%s)\n", path);

	if (chdir(path)) {
		dbg(3, "chdir(%s): %s\n", path, strerror(errno));
		die("Could not chdir() into %s\n", path);
	}
}

char *pjf_pwd(void *mm)
{
	char *ret = mmalloc(PATH_MAX);
	pjf_assert(getcwd(ret, PATH_MAX));
	return ret;
}

void pjf_parsepath(const char *path, tlist *lpath, void *mm)
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

char *pjf_parsedoubleslashes(const char *vcwd, const char *vpath, void *mm)
{
	tlist *list;

	/* get new path in form of a list */
	list = tlist_create(NULL, mm);
	if (vpath[0] != '/') pjf_parsepath(vcwd, list, mm);
	pjf_parsepath(vpath, list, mm);

	return pjf_makepath(list, mm);
}

char *pjf_makepath(tlist *pathparts, void *mm)
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

int pjf_isdir(const char *path)
{
	struct stat dirstat;

	if (stat(path, &dirstat)) return -1;
	if (!S_ISDIR(dirstat.st_mode)) return -2;
	return 1;
}

char *pjf_abspath(const char *path, void *mm)
{
	char cwd[PATH_MAX];

	if (path[0] == '/')
		return mmstrdup(path);
	else
		return mmprintf("%s/%s", getcwd(cwd, sizeof(cwd)), path);
}

const char *pjf_basename(const char *path)
{
	char *l;
	l = strrchr(path, '/');
	return l ? l+1 : path;
}

int pjf_mkdir(const char *path)
{
	mmatic *mm = mmatic_create();
	tlist *list = tlist_create(NULL, mm);
	char *curdir, *part;
	int rc = 0;

	path = pjf_abspath(path, mm);
	pjf_parsepath(path, list, mm);

	curdir = mmalloc(strlen(path) + 2);
	curdir[0] = 0;

	tlist_reset(list);
	while ((part = tlist_iter(list))) {
		strcat(curdir, "/");
		strcat(curdir, part);

		switch (pjf_isdir(curdir)) {
			case  1: continue;           /* exists, is a directory */
			case -2: rc = -1; goto ret;  /* exists, is NOT a directory */
		}

		if (mkdir(curdir, 0755) != 0) {
			rc = -2;
			goto ret;
		}
	}

ret:
	mmatic_free(mm);
	return rc;
}

int pjf_rmdir(const char *path, const char *skip)
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

			newpath = pjf_malloc(strlen(path) + strlen(dirp->d_name) + 2);
			strcpy(newpath, path);
			strcat(newpath, "/");
			strcat(newpath, dirp->d_name);
			upret = pjf_rmdir(newpath, skip);
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

static int vsort(const struct dirent **a, const struct dirent **b) { return strverscmp((*a)->d_name, (*b)->d_name); }
static int filterdots(const struct dirent *d)  { return (!streq(d->d_name, ".") && !streq(d->d_name, "..")); }

tlist *pjf_ls(const char *path, void *mm)
{
	int i, n;
	struct dirent **entries;
	tlist *ret = tlist_create(NULL, mm);

	n = scandir(path, &entries, filterdots, vsort);
	for (i = 0; i < n; i++) {
		tlist_push(ret, mmstrdup(entries[i]->d_name));
		free(entries[i]);
	}

	if (n >= 0) free(entries);
	return ret;
}

int isnumber(const char *str)
{
	int i;

	for (i = 0; str[i]; i++)
		if (!isdigit(str[i]))
			return 0;

	return 1;
}

char *pjf_sanepath(const char *path, void *mm)
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

char *pjf_readfile(const char *path, void *mm)
{
	FILE *fp;
	char *buf, *rbuf;
	uint32_t bigbufsiz, bufsiz;

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

		rbuf = mmatic_resize(rbuf, bigbufsiz);
		buf = rbuf + bufsiz; /* start from half */
	}

	fclose(fp);
	return rbuf;
}

int pjf_writefile(const char *path, const char *s)
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

void pjf_timenow(struct timeval *tv)
{
	if (gettimeofday(tv, NULL) == 0)
		return;

	tv->tv_sec = 0;
	tv->tv_usec = 0;
}

uint32_t pjf_timediff(struct timeval *tv)
{
	static struct timeval tvnow;

	pjf_assert(tv);

	pjf_timenow(&tvnow);

	dbg(14, "pjf_timediff: comparing now=[%u.%06u] vs. then=[%u.%06u]\n",
		(unsigned int) tvnow.tv_sec, (unsigned int) tvnow.tv_usec,
		(unsigned int) tv->tv_sec,   (unsigned int) tv->tv_usec);

	if (tvnow.tv_sec > tv->tv_sec)
		return (tvnow.tv_sec  - tv->tv_sec) * 1000000 - tv->tv_usec + tvnow.tv_usec;
	else if (tvnow.tv_sec == tv->tv_sec &&
	         tvnow.tv_usec > tv->tv_usec)
		return tvnow.tv_usec - tv->tv_usec;
	else
		return 0;
}

void pjf_daemonize(const char *progname, const char *pidfile)
{
	int fd;
	char pwd[PATH_MAX], pid[16];

	if (open(pidfile, O_CREAT | O_TRUNC, 00644) < 0)
		die("PID file not writable: %s", pidfile);

	for (fd = getdtablesize(); fd >= 0; fd--) close(fd);

	if (fork() != 0) _exit(0);
	setsid();
	if (fork() != 0) _exit(0);
	getcwd(pwd, sizeof(pwd));
	chdir("/");

	fd = open("/dev/null", O_RDWR);
	if (fd < 0) _exit(127);

	while ((unsigned) fd < 3) fd = dup(fd);
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
	close(fd);

	openlog((progname) ? progname : "", LOG_PID, LOG_DAEMON);
	debugcb = (void (*)()) syslog;

	pjf_cd(pwd); /* XXX: will die() if fails */

	if (pidfile && !streq(pidfile, "")) {
		snprintf(pid, sizeof(pid), "%u\n", getpid());
		pjf_writefile(pidfile, pid);
	}
}

char *pjf_trim(char *txt)
{
	int i;
	char c;

	if (!txt[0]) return txt;

	while ((c = *txt) && (c == ' ' || c == '\n' || c == '\t')) txt++;
	for (i = 0; txt[i]; i++);
	for (i -= 1; i > 0 && (c = txt[i]) && (c == ' ' || c == '\n' || c == '\t'); i--);
	txt[i + 1] = '\0';

	return txt;
}
