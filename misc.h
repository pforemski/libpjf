/*
 * This file is part of libasn
 * Copyright (C) 2005-2008 by ASN <http://www.asn.pl/>
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

#ifndef _MISC_H_
#define _MISC_H_

#include <stdlib.h>
#include <libasn/tlist.h>
#include <libasn/mmatic.h>

/** Debugging level for dbg() */
extern int debug;

/** Function to call instead of fprintf() for logging a message */
extern void (*debugcb)(const char *msg);

/** I could never understand why there is no such macro by default */
#define streq(a, b) (strcmp((a), (b)) == 0)

/** Prints debugging message if debug <= level
 *
 * @param level message debug level
 * @param dbg message to show, in printf-style format
 */
#ifndef NODEBUG
void dbg(int level, char *dbg, ...);
#else
#define dbg(...) do { } while(0)
#endif

/** Abnormally terminates the program with abort()
 * @param file __FILE__
 * @param line __LINE__
 * @param msg  optional (!= NULL) message format + args to show on stderr
 */
void _die(const char *file, unsigned int line, char *msg, ...);
#define die(msg, ...) (_die(__FILE__, __LINE__, msg, __VA_ARGS__))

/** chdir() or die()
 *
 * @param path directory path
 */
void asn_cd(const char *path);

/** Checks if file exists
 *
 * @param  path path to file/dir
 * @retval  1   exists, is a file
 * @retval  2   exists, is a link
 * @retval -1   does not exist
 * @retval -2   exists, but is not a file nor a link
 */
int asn_isfile(const char *path);

/** Parse path into a tlist
 *
 * @param path   path to parse
 * @param lpath  a tlist to save in (already initialized)
 */
void asn_parsepath(const char *path, tlist *lpath, mmatic *mm);

/** Parse "../" in paths, trim double slashes ("//"), ie. a realpath() on
 * virtual paths
 *
 * @param vcwd   absolute virtual current working directory
 * @param vpath  virtual path to translate, may be relative
 */
char *asn_parsedoubleslashes(const char *vcwd, const char *vpath, mmatic *mm);

/** Creates a path from a asn_parsepath() list
 * @param pathparts list from asn_parsepath()
 */
char *asn_makepath(tlist *pathparts, mmatic *mm);

/** Check if path is a directory
 *
 * @param path   path to check
 * @retval 1     is a dir
 * @retval -1    does not exist
 * @retval -2    exists, but not a dir
 */
int asn_isdir(const char *path);

/** mkdir(1) -p
 * @param path   path to create
 * @param filter if !NULL, a function which will stop asn_mkdir() if it returns 1
 * @retval 0 error
 * @retval 1 success
 */
int asn_mkdir(const char *path, mmatic *mm, int (*filter)(const char *part));

/** rmdir(1) -r
 * @param path  path to remove
 * @param skip  names to skip (optional)
 * @retval 0 error
 * @retval 1 success
 */
int asn_rmdir(const char *path, const char *skip);

/** Returns 1 if str is a number, 0 otherwise */
int isnumber(const char *str);

/** Fast double slash trimmer
 * @param path   path to remove "//"s in
 */
char *asn_sanepath(const char *path, mmatic *mm);

/** Read whole file
 * @param path   path to file
 * @retval null  fopen() failed
 */
char *asn_readfile(const char *path, mmatic *mm);

/** Write file at once - simple wrapper around fputs()
 * @param path   path to file
 * @param s      what to write
 * @retval -1    fopen() failed
 * @return       fopen()s return value (number of bytes written or EOF)
 */
int asn_writefile(const char *path, const char *s);

/** Return current UNIX timestamp in miliseconds */
unsigned int asn_utimestamp();

/** Return current UNIX timestamp */
#define asn_timestamp() (asn_utimestamp() / 1000)

#endif /* _MISC_H_ */
