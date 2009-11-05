/*
 * Header file for manipulation of ptys and utmp entries.
 *
 * Copyright 1995 by the Massachusetts Institute of Technology.
 *
 * 
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of
 * M.I.T. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability
 * of this software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 * 
 */

#ifndef __LIBPTY_H__
#include <stddef.h>

/* Constants for pty_update_utmp */
#define PTY_LOGIN_PROCESS 0
#define PTY_USER_PROCESS 1
#define PTY_DEAD_PROCESS 2

/* Flags to update_utmp */
#define PTY_TTYSLOT_USABLE (0x1)
#define PTY_UTMP_USERNAME_VALID (0x2)

/* Return codes */
#define PTY_GETPTY_STREAMS 1
#define PTY_GETPTY_FSTAT 2
#define PTY_GETPTY_NOPTY 3
#define PTY_GETPTY_SLAVE_TOOLONG 4
#define PTY_OPEN_SLAVE_OPENFAIL 5
#define PTY_OPEN_SLAVE_CHMODFAIL 6
#define PTY_OPEN_SLAVE_NOCTTY 7
#define PTY_OPEN_SLAVE_CHOWNFAIL 8
#define PTY_OPEN_SLAVE_LINE_PUSHFAIL 9
#define PTY_OPEN_SLAVE_PUSH_FAIL 10
#define PTY_OPEN_SLAVE_REVOKEFAIL 11
#define PTY_UPDATE_UTMP_PROCTYPE_INVALID 12
#define PTY_OPEN_SLAVE_TOOSHORT 13

long pty_init(void);
long pty_getpty ( int *fd, char *slave, size_t slavelength);

long pty_open_slave (const char *slave, int *fd);
long pty_open_ctty (const char *slave, int *fd);

long pty_initialize_slave ( int fd);
long pty_update_utmp(int process_type, int pid, const char *user,
		     const char *tty_line, const char *host, int flags);

long pty_logwtmp(const char *tty, const char *user, const char *host);

long pty_cleanup(char *slave, int pid, int update_utmp);

#ifndef SOCK_DGRAM
struct sockaddr;
#endif

long pty_make_sane_hostname(const struct sockaddr *, size_t, int, int,
			    char **);

const char *pty_error_message(long code);

#define __LIBPTY_H__
#endif
