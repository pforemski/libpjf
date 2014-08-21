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

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "lib.h"

pid_t pjf_fork(const char *cmd, const char *args, thash *env,
		int *fd_stdin, int *fd_stdout, int *fd_stderr)
{
	int i, fd_in[2], fd_out[2], fd_err[2];
	pid_t child_pid;
	char *key, *val;

	if (pipe(fd_in) < 0) return 0;
	if (pipe(fd_out) < 0) return 0;
	if (pipe(fd_err) < 0) return 0;

	child_pid = fork();
	if (child_pid < 0) return 0;

	if (child_pid == 0) { /* child */
		char buf[BUFSIZ];

		/* set environment */
		if (env) {
			thash_iter_loop(env, key, val) {
				snprintf(buf, sizeof buf, "%s=%s", key, val);
				putenv(buf);
			}
		}

		/* close all unneeded fds */
		for (i = getdtablesize()-1; i >= 0; i--) {
			if ((i == fd_in[0]) || (i == fd_out[1]) || (i == fd_err[1]))
				continue;
			close(i);
		}

		if (dup2(fd_in[0],  0) == -1) _exit(123);
		if (dup2(fd_out[1], 1) == -1) _exit(124);
		if (dup2(fd_err[1], 2) == -1) _exit(125);

		close(fd_in[0]);
		close(fd_out[1]);
		close(fd_err[1]);

		snprintf(buf, sizeof buf, "%s %s", cmd, args ? args : "");
		execl("/bin/sh", "sh", "-c", "--", buf, NULL);

		_exit(127);
	}

	/* parent */
	dbg(7, "forked PID %u\n", child_pid);

	close(fd_in[0]);
	close(fd_out[1]);
	close(fd_err[1]);

	if (fd_stdout) *fd_stdout = fd_out[0]; else close(fd_out[0]);
	if (fd_stderr) *fd_stderr = fd_err[0]; else close(fd_err[0]);
	if (fd_stdin)  *fd_stdin = fd_in[1]; else close(fd_in[1]);

	return child_pid;
}

int pjf_wait(pid_t child)
{
	int status;

	if (!child) return -1;

	dbg(7, "waiting for PID %u\n", child);
	waitpid(child, &status, 0);

	return
		(WIFEXITED(status))   ? WEXITSTATUS(status) :
		(WIFSIGNALED(status)) ? 200 + WTERMSIG(status) :
		-2;
}

pid_t pjf_waitany(int *code)
{
	pid_t pid;
	int status;

	/* pid == 0 means wait for any child process whose process group ID is equal to that of the calling process */
	pid = waitpid(0, &status, WNOHANG);
	if (pid < 1) return 0;

	if (code) *code =
		(WIFEXITED(status))   ? WEXITSTATUS(status) :
		(WIFSIGNALED(status)) ? 200 + WTERMSIG(status) :
		-2;

	return pid;
}

#define CHECKRC2(arg) if (rc < 0) { e = 1; dbg(4, "%s: %m\n", arg); }

int pjf_cmd(const char *cmd, const char *args, thash *env, xstr *in, xstr *out, xstr *err)
{
	int rc = -1, e = 0, pin = 0, pout = 0, perr = 0;
	pid_t child;
	char buf[8192];

	if (!(child = pjf_fork(cmd, args, env, &pin, &pout, &perr))) {
		e = 1; dbg(2, "pjf_fork() failed\n");
		goto end;
	}

	/* write stdin to child from in */
	if (in) {
		rc = write(pin, xstr_string(in), xstr_length(in));
		CHECKRC2("write(pin)");
	}
	close(pin);

	/* read stdout of child to out */
	if (out) {
		while ((rc = read(pout, buf, sizeof(buf))) > 0)
			xstr_append_size(out, buf, rc);

		CHECKRC2("read(pout)");
		if (xstr_length(out)) dbg(6, "stdout: %s", xstr_string(out));
	}
	close(pout);

	/* read stderr of child to err */
	if (err) {
		while ((rc = read(perr, buf, sizeof(buf))) > 0)
			xstr_append_size(err, buf, rc);

		CHECKRC2("read(perr)");
		if (xstr_length(err)) dbg(6, "stderr: %s", xstr_string(err));
	}
	close(perr);

	rc = pjf_wait(child);
	dbg((rc) ? 2 : 5, "error code: %d\n", rc);

end:
	return (e) ? -1 : rc;
}
