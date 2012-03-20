/*
 * This file is part of libpjf
 * Copyright (C) 2011 Paweł Foremski <pawel@foremski.pl>
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
void _dbg(const char *file, unsigned int line, const char *fn, int level, const char *fmt, ...)
{
	int i;
	va_list args;
	static char buf[BUFSIZ];
	static struct timeval now, tv, first = { 0, 0};
	static bool nl = true;

	if (level > debug) return;

	va_start(args, fmt);
	if ((void (*)()) debugcb == (void (*)()) syslog) {
		buf[0] = '\0';
		i = 0;

		if (nl && level >= 0) {
			if (debug >= 10) {
				snprintf(buf, sizeof buf, "%s:%u %s(): ", file, line, fn);
			} else if (debug > 5 || level >= 0) {
				snprintf(buf, sizeof buf, "%s(): ", fn);
			}

			i = strlen(buf);
		}

		vsnprintf(buf + i, (sizeof buf) - i - 1, fmt, args);
		syslog(LOG_INFO, "%s", buf);
	} else {
		if (nl && level >= 0) {
			if (debug > 5) {
				gettimeofday(&now, NULL);
				if (first.tv_sec == 0) {
					first.tv_sec  = now.tv_sec;
					first.tv_usec = now.tv_usec;
				}
				timersub(&now, &first, &tv);
			}

			if (debug >= 10) {
				snprintf(buf, sizeof(buf), "[%3u.%03u] %s:%u %s(): ",
					(uint32_t) tv.tv_sec, (uint32_t) tv.tv_usec / 1000, file, line, fn);
			} else if (debug > 5) {
				snprintf(buf, sizeof(buf), "[%3u.%03u] %s(): ",
					(uint32_t) tv.tv_sec, (uint32_t) tv.tv_usec / 1000, fn);
			} else {
				snprintf(buf, sizeof(buf), "%s(): ", fn);
			}

			i = strlen(buf);
		} else {
			buf[0] = '\0';
			i = 0;
		}

		vsnprintf(buf + i, (sizeof buf) - i - 1, fmt, args);

		if (debugcb)
			(*debugcb)(buf);
		else
			fputs(buf, stderr);
	}

	i = strlen(buf);
	nl = (i > 0 && buf[i - 1] == '\n');

	va_end(args);
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

int pjf_isdir(const char *path)
{
	struct stat dirstat;

	if (stat(path, &dirstat)) return -1;
	if (!S_ISDIR(dirstat.st_mode)) return -2;
	return 1;
}

void pjf_parsepath(const char *path, tlist *lpath, void *mm)
{
	char *part, *ptr;

	path = mmatic_strdup(mm, path);

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

char *pjf_abspath(const char *path, void *mm)
{
	char cwd[PATH_MAX];

	if (path[0] == '/')
		return mmatic_strdup(mm, path);
	else
		return mmatic_sprintf(mm,
			"%s/%s", getcwd(cwd, sizeof(cwd)), path);
}

const char *pjf_basename(const char *path)
{
	char *l;
	l = strrchr(path, '/');
	return l ? l+1 : path;
}

int pjf_mkdir_mode(const char *path, int mode)
{
	mmatic *mm = mmatic_create();
	tlist *list = tlist_create(NULL, mm);
	char *curdir, *part;
	int rc = 0;

	path = pjf_abspath(path, mm);
	pjf_parsepath(path, list, mm);

	curdir = mmatic_alloc(mm, strlen(path) + 2);
	curdir[0] = 0;

	tlist_reset(list);
	while ((part = tlist_iter(list))) {
		strcat(curdir, "/");
		strcat(curdir, part);

		switch (pjf_isdir(curdir)) {
			case  1: continue;           /* exists, is a directory */
			case -2: rc = -1; goto ret;  /* exists, is NOT a directory */
		}

		if (mkdir(curdir, mode) != 0) {
			rc = -2;
			goto ret;
		}

		chmod(curdir, mode);
	}

ret:
	mmatic_destroy(mm);
	return rc;
}

int pjf_rmdir(const char *path, const char *skip)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *subd;
	char *newpath;
	int upret;

	if (lstat(path, &statbuf) < 0)
		return 0;

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

static int vsort(const struct dirent **a, const struct dirent **b)
	{ return strverscmp((*a)->d_name, (*b)->d_name); }

static int filterdots(const struct dirent *d)
	{ return (!streq(d->d_name, ".") && !streq(d->d_name, "..")); }

tlist *pjf_ls(const char *path, void *mm)
{
	int i, n;
	struct dirent **entries;
	tlist *ret = tlist_create(NULL, mm);

	n = scandir(path, &entries, filterdots, vsort);
	for (i = 0; i < n; i++) {
		tlist_push(ret, mmatic_strdup(mm, entries[i]->d_name));
		free(entries[i]);
	}

	if (n >= 0)
		free(entries);

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
	rbuf = buf = mmatic_alloc(mm, bufsiz);

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

int pjf_copyfile(const char *src, const char *dst)
{
	struct stat stat_src;
	int fd_src, fd_dst, rc;
	char buf[8192];

	/* open source */
	if (stat(src, &stat_src) != 0)
		return -1;

	fd_src = open(src, O_RDONLY);
	if (fd_src < 0)
		return -2;

	/* open destination
	 * copy access mode of source if needed */
	fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, stat_src.st_mode);
	if (fd_dst < 0)
		return -3;

	/* copy */
	while ((rc = read(fd_src, buf, sizeof buf)) > 0) {
		if (write(fd_dst, buf, rc) != rc)
			return -5;
	}

	if (rc < 0)
		return -4;

	/* close */
	close(fd_dst);
	close(fd_src);

	return 0;
}

void pjf_timenow(struct timeval *tv)
{
	if (gettimeofday(tv, NULL) == 0)
		return;

	tv->tv_sec = 0;
	tv->tv_usec = 0;
}

uint32_t pjf_timediff(struct timeval *a, struct timeval *b)
{
	pjf_assert(a);
	pjf_assert(b);

	dbg(14, "pjf_timediff: comparing now=[%u.%06u] vs. then=[%u.%06u]\n",
		(unsigned int) a->tv_sec, (unsigned int) a->tv_usec,
		(unsigned int) b->tv_sec,   (unsigned int) b->tv_usec);

	if (a->tv_sec > b->tv_sec)
		return (a->tv_sec  - b->tv_sec) * 1000000 - b->tv_usec + a->tv_usec;
	else if (a->tv_sec == b->tv_sec &&
	         a->tv_usec > b->tv_usec)
		return a->tv_usec - b->tv_usec;
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

	if (chdir(pwd)) {
		dbg(3, "chdir(%s): %s\n", pwd, strerror(errno));
		die("Could not chdir() into %s\n", pwd);
	}

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
