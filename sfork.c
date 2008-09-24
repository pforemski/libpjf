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
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "misc.h"
#include "thash.h"
#include "mmatic.h"
#include "sfork.h"

pid_t asn_fork(const char *cmd, const char *args, thash *env,
		int *fd_stdin, int *fd_stdout, int *fd_stderr)
{
	int i, rc, script = 0;
	int fd_in[2], fd_in2[2], fd_out[2], fd_err[2];
	pid_t child_pid;
	char *key, *val;

	dbg(10, "asn_fork('%s', '%s')\n", cmd, args);

	if (!strncmp(cmd, "#!/bin/sh", 9)) {
		dbg(6, "asn_fork(): interpreting as inline shell script\n");
		script = 1;
	}

	if (pipe(fd_in) < 0) return 0;
	dbg(11, "asn_fork(): pipe(fd_in): %d %d\n", fd_in[0], fd_in[1]);

	if (pipe(fd_out) < 0) return 0;
	dbg(11, "asn_fork(): pipe(fd_out): %d %d\n", fd_out[0], fd_out[1]);

	if (pipe(fd_err) < 0) return 0;
	dbg(11, "asn_fork(): pipe(fd_err): %d %d\n", fd_err[0], fd_err[1]);

	if (script) {
		if (pipe(fd_in2) < 0) return 0;
		dbg(11, "asn_fork(): pipe(fd_in2): %d %d\n", fd_in2[0], fd_in2[1]);
	}

	child_pid = fork();
	if (child_pid < -1) return 0;

	if (child_pid == 0) { /* child */
		/* set environment */
		if (env) {
			thash_reset(env);
			while ((val = thash_iter(env, &key)))
				putenv(tmprintf("%s=%s", key, val));
		}

		/* close all unneeded fds */
		for (i = getdtablesize()-1; i >= 0; i--) {
			if ((i ==  fd_in[0]) ||
			    (i == fd_out[1]) ||
			    (i == fd_err[1]) ||
			    (script && (i == fd_in2[0]))
			) continue;

			if (!close(i)) /* XXX: after closing(2) dbg() wont work */
				dbg(11, "asn_fork(): closed FD %d\n", i);
		}

		if (dup2(fd_in[0],  0) == -1) _exit(123);
		if (dup2(fd_out[1], 1) == -1) _exit(124);
		if (dup2(fd_err[1], 2) == -1) _exit(125);
		if (script)
			if (dup2(fd_in2[0], 3) == -1) _exit(126);

		if (script)
			execl("/bin/sh", "sh", "-s", "--", args, NULL);
		else
			execl("/bin/sh", "sh", "-c", "--", cmd, args, NULL);

		_exit(127);
	}

	/* parent */
	dbg(7, "asn_fork(): forked PID %u\n", child_pid);

	close(fd_in[0]);
	close(fd_out[1]);
	close(fd_err[1]);

	if (fd_stdout) *fd_stdout = fd_out[0]; else close(fd_out[0]);
	if (fd_stderr) *fd_stderr = fd_err[0]; else close(fd_err[0]);

	/* write cmd to stdin, swap stdin<->fd 3 */
	if (script) {
		close(fd_in2[0]);

#		define CHECKRC(a) if (rc < 0) { dbg(4, "asn_fork(): write(%s): %m\n", (a)); goto writefailed; }
		rc = write(fd_in[1], "#!/bin/sh\n", 10); CHECKRC("shabang");
//		rc = write(fd_in[1], "exec <&3\n", 9); CHECKRC("fd3 hack"); /* TODO: BUG: does not work */
		rc = write(fd_in[1], cmd, strlen(cmd) + 1); CHECKRC("command");

writefailed:
		if (fd_stdin) *fd_stdin = fd_in2[1]; else close(fd_in2[1]);
		close(fd_in[1]);
	}
	else {
		if (fd_stdin) *fd_stdin = fd_in[1]; else close(fd_in[1]);
	}

	return child_pid;
}

int asn_wait(pid_t child)
{
	int status;

	if (!child) return 128;

	dbg(7, "asn_wait(): waiting for PID %u\n", child);
	waitpid(child, &status, 0);

	return
		(WIFEXITED(status))   ? WEXITSTATUS(status) :
		(WIFSIGNALED(status)) ? 200 + WTERMSIG(status) :
		129;
}

pid_t asn_waitany(int *code)
{
	int pid, status;

	pid = waitpid(0, &status, WNOHANG);
	if (pid < 1) { if (code) *code = 130; return 0; }

	if (code) *code =
		(WIFEXITED(status))   ? WEXITSTATUS(status) :
		(WIFSIGNALED(status)) ? 200 + WTERMSIG(status) :
		131;

	return pid;
}

int asn_cmd(const char *cmd, const char *args, thash *env,
	char *in,  int inlen,
	char *out, int outlen,
	char *err, int errlen)
{
	int rc = -1, e = 0, c, pin = 0, pout = 0, perr = 0;
	pid_t child;

	if (!(child = asn_fork(cmd, args, env, &pin, &pout, &perr))) {
		e = 1; dbg(2, "asn_cmd(): asn_fork() failed\n");
		goto end;
	}

	/* write stdin to child from in */
	if (in) {
		rc = write(pin, in, inlen);
#		define CHECKRC2(arg) if (rc < 0) { e = 1; dbg(4, "asn_cmd(): %s: %m\n", arg); }
		CHECKRC2("write(pin)");
	}
	close(pin);

	/* read stdout of child to out */
	if (out) {
		for (c = 0; (rc = read(pout, out+c, outlen-c-1)) > 0; c += rc); out[c] = 0;
		CHECKRC2("read(pout)");
		if (out[0]) dbg(6, "asn_cmd(): stdout: %s", out);
	}
	close(pout);

	/* read stderr of child to err */
	if (err) {
		for (c = 0; (rc = read(perr, err+c, errlen-c-1)) > 0; c += rc); err[c] = 0;
		CHECKRC2("read(perr)");
		if (err[0]) dbg(2, "asn_cmd(): stderr: %s", err);
	}
	close(perr);

	rc = asn_wait(child);
	dbg((rc) ? 2 : 5, "asn_cmd(): error code: %d\n", rc);

end:
	return (e) ? -1 : rc;
}
