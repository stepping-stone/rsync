/*
 * Routines used to setup various kinds of inter-process pipes.
 *
 * Copyright (C) 1996-2000 Andrew Tridgell
 * Copyright (C) 1996 Paul Mackerras
 * Copyright (C) 2001, 2002 Martin Pool <mbp@samba.org>
 * Copyright (C) 2004, 2005, 2006 Wayne Davison
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "rsync.h"

extern int am_sender;
extern int am_server;
extern int blocking_io;
extern int filesfrom_fd;
extern mode_t orig_umask;
extern char *logfile_name;
extern struct chmod_mode_struct *chmod_modes;

/**
 * Create a child connected to us via its stdin/stdout.
 *
 * This is derived from CVS code
 *
 * Note that in the child STDIN is set to blocking and STDOUT
 * is set to non-blocking. This is necessary as rsh relies on stdin being blocking
 *  and ssh relies on stdout being non-blocking
 *
 * If blocking_io is set then use blocking io on both fds. That can be
 * used to cope with badly broken rsh implementations like the one on
 * Solaris.
 **/
pid_t piped_child(char **command, int *f_in, int *f_out)
{
	SECURITY_ATTRIBUTES saAttr;
	HANDLE hChildStdinRd = 0;
	HANDLE hChildStdinWr = 0;
	HANDLE hChildStdoutRd = 0;
	HANDLE hChildStdoutWr = 0;
	BOOL bFuncRetn;
	char buf[1024];
	char** args;

	if (verbose >= 2) {
		print_child_argv(command);
	}

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (! CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 65536))
	{
		rsyserr(FERROR, errno, "pipe");
		exit_cleanup(RERR_IPC);
	}
	SetHandleInformation( hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);

	if (! CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 65536))
	{
		rsyserr(FERROR, errno, "pipe");
		exit_cleanup(RERR_IPC);
	}
	SetHandleInformation( hChildStdinWr, HANDLE_FLAG_INHERIT, 0);

	memset(buf, 0, sizeof(buf)/sizeof(char));
	args=command;
	while (*args)
	{
		if (strlen(buf) + strlen(*args) + 3 < sizeof(buf)/sizeof(char))
		{
			strcat(buf, "\"");
			strcat(buf, *args);
			strcat(buf, "\" ");
		}
		args++;
	}

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	siStartInfo.hStdOutput = hChildStdoutWr;
	siStartInfo.hStdInput = hChildStdinRd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	 bFuncRetn = CreateProcess(NULL,
			buf,     // command line
			NULL,          // process security attributes
			NULL,          // primary thread security attributes
			TRUE,          // handles are inherited
			0,             // creation flags
			NULL,          // use parent's environment
			NULL,          // use parent's current directory
			&siStartInfo,  // STARTUPINFO pointer
			&piProcInfo);  // receives PROCESS_INFORMATION

	SetHandleInformation( hChildStdinWr, HANDLE_FLAG_INHERIT, 1);
	SetHandleInformation( hChildStdoutRd, HANDLE_FLAG_INHERIT, 1);

	SetHandleInformation( hChildStdinRd, HANDLE_FLAG_INHERIT, 0);
	SetHandleInformation( hChildStdoutWr, HANDLE_FLAG_INHERIT, 0);

	if (!CloseHandle(hChildStdoutWr) || !CloseHandle(hChildStdinRd))
	{
		rsyserr(FERROR, errno, "Failed to close");
		exit_cleanup(RERR_IPC);
	}

	*f_in = _open_osfhandle((intptr_t) hChildStdoutRd, O_RDONLY);
	*f_out = _open_osfhandle((intptr_t) hChildStdinWr, 0);

	return (pid_t) piProcInfo.hProcess;
}

/* This function forks a child which calls child_main().  First,
 * however, it has to establish communication paths to and from the
 * newborn child.  It creates two socket pairs -- one for writing to
 * the child (from the parent) and one for reading from the child
 * (writing to the parent).  Since that's four socket ends, each
 * process has to close the two ends it doesn't need.  The remaining
 * two socket ends are retained for reading and writing.  In the
 * child, the STDIN and STDOUT file descriptors refer to these
 * sockets.  In the parent, the function arguments f_in and f_out are
 * set to refer to these sockets. */
pid_t local_child(int argc, char **argv, int *f_in, int *f_out,
		  int (*child_main)(int, char*[]))
{
	pid_t pid;
	int to_child_pipe[2];
	int from_child_pipe[2];

	/* The parent process is always the sender for a local rsync. */
	assert(am_sender);

	if (fd_pair(to_child_pipe) < 0 ||
	    fd_pair(from_child_pipe) < 0) {
		rsyserr(FERROR, errno, "pipe");
		exit_cleanup(RERR_IPC);
	}

	pid = do_fork();
	if (pid == -1) {
		rsyserr(FERROR, errno, "fork");
		exit_cleanup(RERR_IPC);
	}

	if (pid == 0) {
		am_sender = 0;
		am_server = 1;
		filesfrom_fd = -1;
		chmod_modes = NULL; /* Let the sending side handle this. */

		if (dup2(to_child_pipe[0], STDIN_FILENO) < 0 ||
		    close(to_child_pipe[1]) < 0 ||
		    close(from_child_pipe[0]) < 0 ||
		    dup2(from_child_pipe[1], STDOUT_FILENO) < 0) {
			rsyserr(FERROR, errno, "Failed to dup/close");
			exit_cleanup(RERR_IPC);
		}
		if (to_child_pipe[0] != STDIN_FILENO)
			close(to_child_pipe[0]);
		if (from_child_pipe[1] != STDOUT_FILENO)
			close(from_child_pipe[1]);
		child_main(argc, argv);
	}

	/* Let the client side handle this. */
	if (logfile_name) {
		logfile_name = NULL;
		logfile_close();
	}

	if (close(from_child_pipe[1]) < 0 ||
	    close(to_child_pipe[0]) < 0) {
		rsyserr(FERROR, errno, "Failed to close");
		exit_cleanup(RERR_IPC);
	}

	*f_in = from_child_pipe[0];
	*f_out = to_child_pipe[1];

	return pid;
}
