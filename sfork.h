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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SFORK_H_
#define _SFORK_H_

#include "lib.h"

/**
 * @file sfork.h
 * @note catch or ignore SIGPIPE!
 */

/** Fork and run a command in shell (ie. parsing the arguments in command string)
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
pid_t pjf_fork(const char *cmd, const char *args, thash* env,
		int *fd_stdin, int *fd_stdout, int *fd_stderr);

/** Sync wait for termination of given child
 *
 * @param child           PID of child
 * @return                program exit code
 * @retval -1             child=0
 * @retval -2             other error
 * @retval 200 + SIGNUM   child terminated by signal SIGNUM
 */
int pjf_wait(pid_t child);

/** Async check for termination of any child
 *
 * @param code      store exit code here if not null (see retval of pjf_wait())
 * @return          child pid
 * @retval 0        no child exited so far
 */
pid_t pjf_waitany(int *code);

/** Wrapper around pjf_fork()
 *
 * @param cmd      command to execute (may be a shell script code)
 * @param args     optional arguments
 * @param envh     optional environmental variables to set
 * @param in       optional input for the command
 * @param out      optional buffer for output of the command
 * @param err      optional buffer for errors of the command
 *
 * @returns exit code of the command
 * @retval  -1      error in pjf_cmd()
 */
int pjf_cmd(const char *cmd, const char *args, thash *env, xstr *in, xstr *out, xstr *err);

#endif /* _SFORK_H_*/
