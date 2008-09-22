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

#ifndef _SFORK_H_
#define _SFORK_H_

#include <sys/types.h>
#include <libasn/thash.h>

/**
 * @file sfork.h
 * @note catch or ignore SIGPIPE!
 */

/** Fork and run a command in shell (ie. parsing the arguments in command string)
 *
 * If cmd starts with #!/bin/sh, makes evil black magic and executes it as "inline"
 * shell scripts
 *
 * @param cmd          command to run, with optional arguments
 * @param args         additional optional arguments (may be null)
 * @param env          thash to export as process environment
 * @param fd_stdin     for pipe to stdin of the child [write-only pipe]
 * @param fd_stdout    for pipe to stdout of the child [read-only pipe]
 * @param fd_stderr    for pipe to stderr of the child [read-only pipe]
 *
 * @return child PID
 * @retval 0 error
 */
pid_t sfork(const char *cmd, const char *args, thash* env,
		int *fd_stdin, int *fd_stdout, int *fd_stderr);

/** Waits for termination of child
 *
 * @param child           PID of child
 * @return                exit code
 * @retval 128            child=0
 * @retval 130+termsignal child terminated
 * @retval 129            other error
 */
int swait(pid_t child);

/** Waits for termination of any child
 *
 * @param code      store exit code here if not null
 * @return          child pid
 * @retval 0        no child exited so far
 */
pid_t swaitany(int *code);

/** Wrapper around sfork()
 *
 * @param cmd      command to execute (may be a shell script code)
 * @param args     optional arguments
 * @param envh     optional environmental variables to set
 * @param in       optional input for the command
 * @param inlen    length of in
 * @param out      optional buffer for output of the command
 * @param outlen   length of out
 * @param err      optional buffer for errors of the command
 * @param errlen   length of err
 *
 * @returns exit code of the command
 * @retval  -1      error in sexec()
 */
int sexec(const char *cmd, const char *args,
	thash *envh,
	char *in, int inlen,
	char *out, int outlen,
	char *err, int errlen);

#endif /* _SFORK_H_*/
