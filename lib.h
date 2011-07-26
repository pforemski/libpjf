/*
 * This file is part of libpjf
 * Copyright (C) 2005-2009 ASN Sp. z o.o.
 * Copyright (C) 2011 Paweł Foremski
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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LIBPJF_H_
#define _LIBPJF_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

/*****************************************************************************/

/** Debugging level for dbg() */
extern int debug;

/** Function to call instead of fprintf() for logging a message */
extern void (*debugcb)(const char *msg);

/** Prints debugging message if debug <= level
 * @param level message debug level
 * @param dbg message to show, in printf-style format */
#ifndef NODEBUG
void _dbg(const char *file, unsigned int line, const char *fn, int level, char *fmt, ...);
#define dbg(...) _dbg(__FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define dbg(...) do { } while(0)
#endif

/** Abnormally terminates the program with abort()
 * @param file __FILE__
 * @param line __LINE__
 * @param fmt optional (!= NULL) message format + args to show on stderr (printf-like) */
void _die(const char *file, unsigned int line, const char *fn, char *fmt, ...);
#define die(...) (_die(__FILE__, __LINE__, __func__, __VA_ARGS__))
#define pjf_assert(a) do { if(!(a)) die("Assertion failed\n"); } while(0);
#define die_errno(msg) (die("%s: %s\n", (msg), strerror(errno)))

/** Macro for simple error handling */
#define reterr(val, lvl, msg1, msg2) { dbg((lvl), "%s: %s\n", (msg1), (msg2)); return (val); }
#define reterrno(val, lvl, msg) reterr((val), (lvl), (msg), strerror(errno))

/*****************************************************************************/

/** From linux/stddef.h */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/** Number of elements in array */
#define N(a) (sizeof(a)/sizeof(*a))

/** A generic foreach */
#define foreach(array, ptr) \
	for (typeof((array)+0) (ptr) = (array)+0; (ptr) < (array)+N(array); (ptr) += 1)

/*****************************************************************************/

/** Wrapper around malloc() which prints an error message and exits() with 1 in
 * case of an error
 *
 * @param size same as in stdlib's malloc()
 */
void *pjf_malloc(size_t size);

/** mmatic_printf() using pjf_malloc() */
char *pjf_malloc_printf(const char *fmt, ...);

/*****************************************************************************/

/* All libpjf components included */
#include "thash.h"
#include "mmatic.h"
#include "tlist.h"
#include "tsort.h"
#include "math.h"
#include "regex.h"
#include "xstr.h"
#include "unitype.h"
#include "json.h"
#include "rfc822.h"
#include "encode.h"
#include "utf8.h"

/** I could never understand why there is no such macro by default --pjf */
#define streq(a, b) (strcmp((a), (b)) == 0)

/** Trim string from both sides
 * @note removes ' ', '\t' and '\n'
 * @note modifies txt and returns memory location within it */
char *pjf_trim(char *txt);

/** Checks if all str characters are digits
 * @retval 1 str is a number
 * @retval 0 str is not a number */
int isnumber(const char *str);

/*****************************************************************************/

/** chdir() or die()
 * @param path directory path */
void pjf_cd(const char *path);

/** Returns current working directory */
char *pjf_pwd(void *mm);

/** Checks if file exists
 * @param  path path to file/dir
 * @retval  1   exists, is a file
 * @retval  2   exists, is a link
 * @retval -1   does not exist
 * @retval -2   exists, but is not a file nor a link */
int pjf_isfile(const char *path);

/** Checks if file has any of executable bits set */
bool pjf_isexecutable(const char *path);

/** Checks if file is a FIFO
 * @param  path path to file/dir
 * @retval  1   exists and is a FIFO
 * @retval -1   does not exist
 * @retval -2   exists, but is not a fifo */
int pjf_isfifo(const char *path);

/** Parse path into a tlist
 * @param path   path to parse
 * @param lpath  a tlist to save in (already initialized) */
void pjf_parsepath(const char *path, tlist *lpath, void *mm);

/** Parse "../" in paths, trim double slashes ("//"), ie. a realpath() on
 * virtual paths
 * @param vcwd   absolute virtual current working directory
 * @param vpath  virtual path to translate, may be relative */
char *pjf_parsedoubleslashes(const char *vcwd, const char *vpath, void *mm);

/** Creates a path from a pjf_parsepath() list
 * @param pathparts list from pjf_parsepath() */
char *pjf_makepath(tlist *pathparts, void *mm);

/** Convert relative path to absolute one
 * @param  path  path to convert
 * @param  mm    memory for new path
 * @note always returns the value in given mm */
char *pjf_abspath(const char *path, void *mm);

/** Return last element after "/" in path */
const char *pjf_basename(const char *path);

/** Check if path is a directory
 * See errno for more details on negative return values
 * @param   path   path to check
 * @retval  1      is a dir
 * @retval -1      does not exist
 * @retval -2      exists, but not a dir */
int pjf_isdir(const char *path);

/** mkdir(1) -p
 * @param  path    path to create
 * @param  mode    access mode for new directories
 * @retval -1      error: part of path already exists and is not a directory
 * @retval -2      error: mkdir() see errno
 * @retval 0       success */
int pjf_mkdir_mode(const char *path, int mode);

/** pjf_mkdir_mode() with mode 0755 */
#define pjf_mkdir(path) pjf_mkdir_mode((path), 0755)

/** rm -fr
 * @param path  path to remove
 * @param skip  names to skip (optional)
 * @retval 0 error
 * @retval 1 success */
int pjf_rmdir(const char *path, const char *skip);

/** ls dir
 * @param path  directory to scan
 * @note        automatically skips . and .. */
tlist *pjf_ls(const char *path, void *mm);

/** Fast double slash trimmer
 * @param path   path to remove "//"s in */
char *pjf_sanepath(const char *path, void *mm);

/** Read whole file
 * @param  path  path to file
 * @retval null  fopen() failed */
char *pjf_readfile(const char *path, void *mm);

/** Write file at once - simple wrapper around fputs()
 * @param  path  path to file
 * @param  s     what to write
 * @retval -1    fopen() failed
 * @return       fopen()s return value (number of bytes written or EOF) */
int pjf_writefile(const char *path, const char *s);

/** Copy a file
 * @param src     path to source
 * @param dst     path to destination
 * @retval 0      success
 * @retval -1     stat(src) failed
 * @retval -2     open(src) failed
 * @retval -3     open(dst) failed
 * @retval -4     read(src) failed
 * @retval -5     write(dst) failed
 * @retval <0     error
 */
int pjf_copyfile(const char *src, const char *dst);

/*****************************************************************************/

/** Wrapper around gettimeofday() */
void pjf_timenow(struct timeval *tv);

/** Returns time difference in us vs. current time and the time given in tv */
uint32_t pjf_timediff(struct timeval *tv);

/** Return current UNIX timestamp */
#define pjf_timestamp() ((uint32_t) time(NULL))

/*****************************************************************************/

/** Daemonize a program
 * @param progname    program name to show in syslog messages
 * @param pidfile     path to file where to store the PID in (may be null) */
void pjf_daemonize(const char *progname, const char *pidfile);

/** Frequency -> period conversions */
#define Hz_to_msec(f) (f ? (1.0/f * 1000.0) : 0)

#define msec_to_Hz Hz_to_msec

#endif /* _MISC_H_ */
