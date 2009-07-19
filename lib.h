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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LIBASN_H_
#define _LIBASN_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/** Debugging level for dbg() */
extern int debug;

/** Function to call instead of fprintf() for logging a message */
extern void (*debugcb)(const char *msg);

/** Prints debugging message if debug <= level
 * @param level message debug level
 * @param dbg message to show, in printf-style format */
#ifndef NODEBUG
void dbg(int level, char *dbg, ...);
#else
#define dbg(...) do { } while(0)
#endif

/** Abnormally terminates the program with abort()
 * @param file __FILE__
 * @param line __LINE__
 * @param msg  optional (!= NULL) message format + args to show on stderr */
void _die(const char *file, unsigned int line, char *msg, ...);
#define die(...) (_die(__FILE__, __LINE__, __VA_ARGS__))
#define asnsert(a) do { if(!(a)) die("Assertion failed\n"); } while(0);

/* All libasn components included */
#include "thash.h"
#include "mmatic.h"
#include "tlist.h"
#include "tsort.h"
#include "fifos.h"
#include "math.h"
#include "sfork.h"
#include "regex.h"
#include "xstr.h"
#include "wstr.h"
#include "fc.h"
#include "select.h"
#include "fcml.h"

/** Shortcut for programs using libasn */
#define __USE_LIBASN int debug = 0; void (*debugcb)() = NULL;

/** I could never understand why there is no such macro by default --pjf */
#define streq(a, b) (strcmp((a), (b)) == 0)

/** chdir() or die()
 * @param path directory path */
void asn_cd(const char *path);

/** Returns current working directory */
char *asn_pwd(mmatic *mm);

/** Checks if file exists
 * @param  path path to file/dir
 * @retval  1   exists, is a file
 * @retval  2   exists, is a link
 * @retval -1   does not exist
 * @retval -2   exists, but is not a file nor a link */
int asn_isfile(const char *path);

/** Checks if file has any of executable bits set */
bool asn_isexecutable(const char *path);

/** Checks if file is a FIFO
 * @param  path path to file/dir
 * @retval  1   exists and is a FIFO
 * @retval -1   does not exist
 * @retval -2   exists, but is not a fifo */
int asn_isfifo(const char *path);

/** Parse path into a tlist
 * @param path   path to parse
 * @param lpath  a tlist to save in (already initialized) */
void asn_parsepath(const char *path, tlist *lpath, mmatic *mm);

/** Parse "../" in paths, trim double slashes ("//"), ie. a realpath() on
 * virtual paths
 * @param vcwd   absolute virtual current working directory
 * @param vpath  virtual path to translate, may be relative */
char *asn_parsedoubleslashes(const char *vcwd, const char *vpath, mmatic *mm);

/** Creates a path from a asn_parsepath() list
 * @param pathparts list from asn_parsepath() */
char *asn_makepath(tlist *pathparts, mmatic *mm);

/** Convert relative path to absolute one
 * @param  path  path to convert
 * @param  mm    memory for new path
 * @note always returns the value in given mm */
char *asn_abspath(const char *path, mmatic *mm);

/** Check if path is a directory
 * @param   path   path to check
 * @retval  1      is a dir
 * @retval -1      does not exist
 * @retval -2      exists, but not a dir */
int asn_isdir(const char *path);

/** mkdir(1) -p
 * @param  path    path to create
 * @param  filter  if !NULL, a function to call on each iteration and exit from asn_mkdir() with retval 1 if this
 *                 callback function returns 1, "part" in cb args is the path part were just about to create
 * @retval 0       error
 * @retval 1       success */
int asn_mkdir(const char *path, mmatic *mm, int (*filter)(const char *part));

/** rm -fr
 * @param path  path to remove
 * @param skip  names to skip (optional)
 * @retval 0 error
 * @retval 1 success */
int asn_rmdir(const char *path, const char *skip);

/** ls dir
 * @param path  directory to scan
 * @note        automatically skips . and .. */
tlist *asn_ls(const char *path, mmatic *mm);

/** Checks if all str characters are digits
 * @retval 1 str is a number
 * @retval 0 str is not a number */
int isnumber(const char *str);

/** Fast double slash trimmer
 * @param path   path to remove "//"s in */
char *asn_sanepath(const char *path, mmatic *mm);

/** Read whole file
 * @param  path  path to file
 * @retval null  fopen() failed */
char *asn_readfile(const char *path, mmatic *mm);

/** Write file at once - simple wrapper around fputs()
 * @param  path  path to file
 * @param  s     what to write
 * @retval -1    fopen() failed
 * @return       fopen()s return value (number of bytes written or EOF) */
int asn_writefile(const char *path, const char *s);

/** Return current UNIX timestamp in miliseconds */
unsigned int asn_utimestamp();

/** Return current UNIX timestamp */
#define asn_timestamp() (asn_utimestamp() / 1000)

/** Daemonize a program
 * @param progname    program name to show in syslog messages
 * @param pidfile     path to file where to store the PID in (may be null) */
void asn_daemonize(const char *progname, const char *pidfile);

#endif /* _MISC_H_ */
