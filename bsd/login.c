/*
 *	$Source$
 *	$Author$
 *	$Id$
 */

#ifndef lint
static char rcsid_login_c[] = "$Id$";
#endif lint

/*
 * Copyright (c) 1980, 1987, 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980, 1987, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)login.c	5.25 (Berkeley) 1/6/89";
#endif /* not lint */

/*
 * login [ name ]
 * login -r hostname	(for rlogind)
 * login -h hostname	(for telnetd, etc.)
 * login -f name	(for pre-authenticated login: datakit, xterm, etc.)
 * login -F name	(for pre-authenticated login: datakit, xterm, etc.,
 *			 allows preauthenticated login as root)
 * login -e name	(for pre-authenticated encrypted, must do term
 *			 negotiation)
 * ifdef KRB4
 * login -k hostname (for Kerberos V4 rlogind with password access)
 * login -K hostname (for Kerberos V4 rlogind with restricted access)
 * endif KRB4 
 *
 * only one of: -r -f -e -k -K -F
 * only one of: -r -h -k -K
 */

#include <sys/types.h>
#include <sys/param.h>
#ifdef OQUOTA
#include <sys/quota.h>
#endif
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include <utmp.h>
#include <signal.h>

#if !defined(_AIX)
#include <lastlog.h>
#endif

#include <errno.h>
#ifndef NOTTYENT
#include <ttyent.h>
#endif /* NOTTYENT */
#include <syslog.h>
#include <grp.h>
#include <pwd.h>
#include <setjmp.h>
#include <stdio.h>
#include <strings.h>
#ifdef KRB4
#include <krb.h>
#include <netdb.h>
#include <netinet/in.h>
#ifdef BIND_HACK
#include <arpa/nameser.h>
#include <arpa/resolv.h>
#endif /* BIND_HACK */
#endif /* KRB4 */

#ifdef POSIX
#include <stdlib.h>
#include <termios.h>
#ifdef _AIX
#include <termio.h>
#endif
#endif

#ifdef _IBMR2
#include <usersec.h>
#include <sys/id.h>
#endif

#if defined(_AIX)
#define PRIO_OFFSET 20
#else
#define PRIO_OFFSET 0
#endif

#ifdef UIDGID_T
uid_t getuid();
#define uid_type uid_t
#define gid_type gid_t
#else
int getuid();
#define uid_type int
#define gid_type int
#endif /* UIDGID_T */

#define	TTYGRPNAME	"tty"		/* name of group to own ttys */

#define	MOTDFILE	"/etc/motd"
#define	MAILDIR		"/usr/spool/mail"
#define	NOLOGIN		"/etc/nologin"
#define	HUSHLOGIN	".hushlogin"
#define	LASTLOG		"/usr/adm/lastlog"
#define	BSHELL		"/bin/sh"

#if !defined(OQUOTA) && !defined(QUOTAWARN)
#define QUOTAWARN	"/usr/ucb/quota" /* warn user about quotas */
#endif

#define PROTOTYPE_DIR	"/usr/athena/lib/prototype_tmpuser"
#define TEMP_DIR_PERM	0711

#define NOATTACH	"/etc/noattach"
#define NOCREATE	"/etc/nocreate"
#define NOREMOTE	"/etc/noremote"
#define REGISTER	"/usr/etc/go_register"
#define GET_MOTD	"/bin/athena/get_message"

#define	UT_HOSTSIZE	sizeof(((struct utmp *)0)->ut_host)
#define	UT_NAMESIZE	sizeof(((struct utmp *)0)->ut_name)

#define MAXENVIRON	32

/*
 * This bounds the time given to login.  Not a define so it can
 * be patched on machines where it's too small.
 */
int	timeout = 300;

struct passwd *pwd;
char term[64], *hostname, *username;

#ifndef POSIX
struct sgttyb sgttyb;
struct tchars tc = {
	CINTR, CQUIT, CSTART, CSTOP, CEOT, CBRK
};
struct ltchars ltc = {
	CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT
};
#endif

extern int errno;

#ifdef KRB4
#define KRB_ENVIRON	"KRBTKFILE"	/* Ticket file environment variable */
#define KRB_TK_DIR	"/tmp/tkt_"	/* Where to put the ticket */
#define MAXPWSIZE	128		/* Biggest string accepted for KRB4
					   passsword */

extern char *krb_err_txt[];		/* From libkrb */

AUTH_DAT *kdata = (AUTH_DAT *) NULL;
KTEXT ticket = (KTEXT) NULL;
char tkfile[MAXPATHLEN];
int krbflag = 0;			/* set if tickets have been obtained */
#ifdef SETPAG
int pagflag = 0;			/* true if setpag() has been called */
#endif /* SETPAG */
#endif /* KRB4 */

char *getenv();
char *strsave();
void dofork();

#ifdef POSIX
typedef void sigtype;
#else
typedef int sigtype;
#endif /* POSIX */

#define EXCL_AUTH_TEST if (rflag || kflag || Kflag || eflag || fflag || Fflag ) { \
				fprintf(stderr, \
				    "login: only one of -r, -k, -K, -e, -F, and -f allowed.\n"); \
				exit(1);\
			}

#define EXCL_HOST_TEST if (rflag || kflag || Kflag || hflag) { \
				fprintf(stderr, \
				    "login: only one of -r, -k, -K, and -h allowed.\n"); \
				exit(1);\
			}

main(argc, argv)
	int argc;
	char **argv;
{
	extern int optind;
	extern char *optarg, **environ;
	struct group *gr;
	register int ch, i;
	register char *p;
	int fflag, hflag, pflag, rflag, Fflag, cnt;
	int kflag, Kflag, eflag;
	int quietlog, passwd_req, ioctlval;
	sigtype timedout();
	char *domain, *salt, **envinit, *ttyn, *tty, *ktty;
	char tbuf[MAXPATHLEN + 2];
	char *ttyname(), *stypeof(), *crypt(), *getpass();
	time_t time(), login_time;
	off_t lseek();
#ifdef POSIX
	struct termios tc;
#endif

	(void)signal(SIGALRM, timedout);
	(void)alarm((u_int)timeout);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGINT, SIG_IGN);
	(void)setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
#ifdef OQUOTA
	(void)quota(Q_SETUID, 0, 0, 0);
#endif

	/*
	 * -p is used by getty to tell login not to destroy the environment
	 * -r is used by rlogind to cause the autologin protocol;
 	 * -f is used to skip a second login authentication 
 	 * -F is used to skip a second login authentication, allows login as root 
	 * -e is used to skip a second login authentication, but allows
	 * 	login as root.
	 * -h is used by other servers to pass the name of the
	 * remote host to login so that it may be placed in utmp and wtmp
	 * -k is used by klogind to cause the Kerberos V4 autologin protocol;
	 * -K is used by klogind to cause the Kerberos V4 autologin
	 *    protocol with restricted access.
	 */
	(void)gethostname(tbuf, sizeof(tbuf));
	domain = strchr(tbuf, '.');

	Fflag = fflag = hflag = pflag = rflag = kflag = Kflag = eflag = 0;
	passwd_req = 1;
	while ((ch = getopt(argc, argv, "Ffeh:pr:k:K:")) != EOF)
		switch (ch) {
		case 'f':
			EXCL_AUTH_TEST;
			fflag = 1;
			break;
		case 'F':
			EXCL_AUTH_TEST;
			Fflag = 1;
			break;
		case 'h':
			EXCL_HOST_TEST;
			if (getuid()) {
				fprintf(stderr,
				    "login: -h for super-user only.\n");
				exit(1);
			}
			hflag = 1;
			if (domain && (p = strchr(optarg, '.')) &&
			    strcmp(p, domain) == 0)
				*p = 0;
			hostname = optarg;
			break;
		case 'p':
			pflag = 1;
			break;
		case 'r':
			EXCL_AUTH_TEST;
			EXCL_HOST_TEST;
			if (getuid()) {
				fprintf(stderr,
				    "login: -r for super-user only.\n");
				exit(1);
			}
			/* "-r hostname" must be last args */
			if (optind != argc) {
				fprintf(stderr, "Syntax error.\n");
				exit(1);
			}
			rflag = 1;
			passwd_req = (doremotelogin(optarg) == -1);
			if (domain && (p = strchr(optarg, '.')) &&
			    !strcmp(p, domain))
				*p = '\0';
			hostname = optarg;
			break;
#ifdef KRB4
		case 'k':
		case 'K':
			EXCL_AUTH_TEST;
			EXCL_HOST_TEST;
			if (getuid()) {
				fprintf(stderr,
				    "login: -%c for super-user only.\n", ch);
				exit(1);
			}
			/* "-k hostname" must be last args */
			if (optind != argc) {
				fprintf(stderr, "Syntax error.\n");
				exit(1);
			}
			if (ch == 'K')
			    Kflag = 1;
			else
			    kflag = 1;
			passwd_req = (do_krb_login(optarg,
						   Kflag ? 1 : 0) == -1);
			if (domain && (p = strchr(optarg, '.')) &&
			    !strcmp(p, domain))
				*p = '\0';
			hostname = optarg;
			break;
#endif /* KRB4 */
		case 'e':
			EXCL_AUTH_TEST;
			if (getuid()) {
			    fprintf(stderr,
				    "login: -e for super-user only.\n");
			    exit(1);
			}
			eflag = 1;
			passwd_req = 0;
			break;
		case '?':
		default:
			fprintf(stderr, "usage: login [-fp] [username]\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;
	if (*argv)
		username = *argv;

#if !defined(_AIX)
	ioctlval = 0;
	(void)ioctl(0, TIOCLSET, (char *)&ioctlval);
	(void)ioctl(0, TIOCNXCL, (char *)0);
	(void)fcntl(0, F_SETFL, ioctlval);
#endif
#ifdef POSIX
	(void)tcgetattr(0, &tc);
#else
	(void)ioctl(0, TIOCGETP, (char *)&sgttyb);
#endif

	/*
	 * If talking to an rlogin process, propagate the terminal type and
	 * baud rate across the network.
	 */
	if (eflag)
	    	lgetstr(term, sizeof(term), "Terminal type");
#ifdef POSIX
	if (rflag || kflag || Kflag || eflag)
		doremoteterm(&tc);
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 0;
	tc.c_cc[VERASE] = CERASE;
	tc.c_cc[VKILL] = CKILL;
	tc.c_cc[VEOF] = CEOF;
	tc.c_cc[VINTR] = CINTR;
	tc.c_cc[VQUIT] = CQUIT;
	tc.c_cc[VSTART] = CSTART;
	tc.c_cc[VSTOP] = CSTOP;
	tc.c_cc[VEOL] = CNUL;
	/* The following are common extensions to POSIX */
#ifdef VEOL2
	tc.c_cc[VEOL2] = CNUL;
#endif
#ifdef VSUSP
	tc.c_cc[VSUSP] = CSUSP;
#endif
#ifdef VDSUSP
	tc.c_cc[VDSUSP] = CDSUSP;
#endif
#ifdef VLNEXT
	tc.c_cc[VLNEXT] = CLNEXT;
#endif
#ifdef VREPRINT
	tc.c_cc[VREPRINT] = CRPRNT;
#endif
#ifdef VDISCRD
	tc.c_cc[VDISCRD] = CFLUSH;
#endif
#ifdef VWERSE
	tc.c_cc[VWERSE] = CWERASE;
#endif
	tcsetattr(0, TCSANOW, &tc);
#else
	if (rflag || kflag || Kflag || eflag)
		doremoteterm(&sgttyb);
	sgttyb.sg_erase = CERASE;
	sgttyb.sg_kill = CKILL;
	(void)ioctl(0, TIOCSLTC, (char *)&ltc);
	(void)ioctl(0, TIOCSETC, (char *)&tc);
	(void)ioctl(0, TIOCSETP, (char *)&sgttyb);
#endif

	for (cnt = getdtablesize(); cnt > 2; cnt--)
		(void) close(cnt);

	ttyn = ttyname(0);
	if (ttyn == NULL || *ttyn == '\0')
		ttyn = "/dev/tty??";

	/* This allows for tty names of the form /dev/pts/4 as well */
	if ((tty = strchr(ttyn, '/')) && (tty = strchr(tty+1, '/')))
		++tty;
	else
		tty = ttyn;

	/* For kerberos tickets, extract only the last part of the ttyname */
	if (ktty = strrchr(tty, '/'))
		++ktty;
	else
		ktty = tty;

#ifndef LOG_ODELAY /* 4.2 syslog ... */                      
	openlog("login", 0);
#else
	openlog("login", LOG_ODELAY, LOG_AUTH);
#endif /* 4.2 syslog */

	for (cnt = 0;; username = NULL) {
#ifdef KRB4
		char pp[9], pp2[MAXPWSIZE], *namep;
		int krbval;
		char realm[REALM_SZ];
		int kpass_ok,lpass_ok;
#ifdef NOENCRYPTION
#define read_long_pw_string placebo_read_pw_string
#else
#define read_long_pw_string des_read_pw_string
#endif
		int read_long_pw_string();
#endif /* KRB4 */
#if !defined(_IBMR2)
		ioctlval = 0;
		(void)ioctl(0, TIOCSETD, (char *)&ioctlval);
#endif

		if (username == NULL) {
			fflag = Fflag = 0;
			getloginname();
		}

		if (pwd = getpwnam(username))
			salt = pwd->pw_passwd;
		else
			salt = "xx";

		/* if user not super-user, check for disabled logins */
		if (pwd == NULL || pwd->pw_uid)
			checknologin();

		/*
		 * Disallow automatic login to root; if not invoked by
		 * root, disallow if the uid's differ.
		 */
		if (fflag && pwd) {
			int uid = (int) getuid();

			passwd_req = pwd->pw_uid == 0 ||
			    (uid && uid != pwd->pw_uid);
		}

		/*
		 * Allows automatic login by root.
		 * If not invoked by root, disallow if the uid's differ.
		 */

		if (Fflag && pwd) {
			int uid = (int) getuid();
			passwd_req = uid && uid != pwd->pw_uid;
		}

		/*
		 * If no remote login authentication and a password exists
		 * for this user, prompt for one and verify it.
		 */
		if (!passwd_req || pwd && !*pwd->pw_passwd)
			break;

#ifdef KRB4
		kpass_ok = 0;
		lpass_ok = 0;

		(void) setpriority(PRIO_PROCESS, 0, -4 + PRIO_OFFSET);
		if (read_long_pw_string(pp2, sizeof(pp2)-1, "Password: ", 0)) {
		    /* reading password failed... */
		    (void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
		    goto bad_login;
		}
		if (!pwd)		/* avoid doing useless work */
		    goto bad_login;

		/* Modifications for Kerberos authentication -- asp */
		(void) strncpy(pp, pp2, sizeof(pp));
		pp[8]='\0';
		namep = crypt(pp, pwd->pw_passwd);
		memset (pp, 0, sizeof(pp));	/* To the best of my recollection, Senator... */
		lpass_ok = !strcmp (namep, pwd->pw_passwd);
		
		if (pwd->pw_uid != 0) { /* Don't get tickets for root */

		    if (krb_get_lrealm(realm, 1) != KSUCCESS) {
			(void) strncpy(realm, KRB_REALM, sizeof(realm));
		    }
#ifdef BIND_HACK
		    /* Set name server timeout to be reasonable,
		       so that people don't take 5 minutes to
		       log in.  Can you say abstraction violation? */
		    _res.retrans = 1;
#endif /* BIND_HACK */
#if 0  /* XXX krb5 has defaults; don't do this. */
		    /* Set up the ticket file environment variable */
		    strncpy(tkfile, KRB_TK_DIR, sizeof(tkfile));
		    strncat(tkfile, ktty,
			    sizeof(tkfile) - strlen(tkfile) - 1);
		    (void) setenv(KRB_ENVIRON, tkfile, 1);
		    krb_set_tkt_string(tkfile);
#endif

#ifdef _IBMR2
		    krbval = setuidx(ID_REAL|ID_EFFECTIVE, pwd->pw_uid);
#else
		    krbval = setreuid(pwd->pw_uid, -1);
#endif
		    if (krbval) {
			/* can't set ruid to user! */
			krbval = -1;
			fprintf(stderr,
				"login: Can't set ruid for ticket file.\n");
		    } else
			krbval = krb_get_pw_in_tkt(username, "",
						   realm, "krbtgt",
						   realm,
						   DEFAULT_TKT_LIFE, pp2);
		    memset (pp2, 0, sizeof(pp2));
		    (void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
		    switch (krbval) {
		    case INTK_OK:
			kpass_ok = 1;
			krbflag = 1;
			break;	

		    /* These errors should be silent */
		    /* So the Kerberos database can't be probed */
		    case KDC_NULL_KEY:
		    case KDC_PR_UNKNOWN:
		    case INTK_BADPW:
		    case KDC_PR_N_UNIQUE:
		    case -1:
			break;
		    /* These should be printed but are not fatal */
		    case INTK_W_NOTALL:
			krbflag = 1;
			kpass_ok = 1;
			fprintf(stderr, "Kerberos error: %s\n",
				krb_err_txt[krbval]);
			break;
		    default:
			fprintf(stderr, "Kerberos error: %s\n",
				krb_err_txt[krbval]);
			break;
		    }
		} else {
		    (void) memset (pp2, 0, sizeof(pp2));
		    (void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
		}

		/* Policy: If local password is good, user is good.
		   We really can't trust the Kerberos password,
		   because somebody on the net could spoof the
		   Kerberos server (not easy, but possible).
		   Some sites might want to use it anyways, in
		   which case they should change this line
		   to:
		   if (kpass_ok)
		   */
		if (lpass_ok)
		    break;
bad_login:
		if (krbflag)
		    dest_tkt();		/* clean up tickets if login fails */
#else /* !KRB4 */
		(void) setpriority(PRIO_PROCESS, 0, -4 + PRIO_OFFSET);
		p = crypt(getpass("password:"), salt);
		(void) setpriority(PRIO_PROCESS, 0, 0 + PRIO_OFFSET);
		if (pwd && !strcmp(p, pwd->pw_passwd))
			break;
#endif /* KRB4 */

		printf("Login incorrect\n");
		if (++cnt >= 5) {
			if (hostname)
			    syslog(LOG_ERR,
				"REPEATED LOGIN FAILURES ON %s FROM %.*s, %.*s",
				tty, UT_HOSTSIZE, hostname, UT_NAMESIZE,
				username);
			else
			    syslog(LOG_ERR,
				"REPEATED LOGIN FAILURES ON %s, %.*s",
				tty, UT_NAMESIZE, username);
			(void)ioctl(0, TIOCHPCL, (char *)0);
			sleepexit(1);
		}
	}

	/* committed to login -- turn off timeout */
	(void)alarm((u_int)0);

	/*
	 * If valid so far and root is logging in, see if root logins on
	 * this terminal are permitted.
	 *
	 * We allow authenticated remote root logins (except -r style)
	 */
	if (pwd->pw_uid == 0 && !rootterm(tty) && (passwd_req || rflag)) {
		if (hostname)
			syslog(LOG_ERR, "ROOT LOGIN REFUSED ON %s FROM %.*s",
			    tty, UT_HOSTSIZE, hostname);
		else
			syslog(LOG_ERR, "ROOT LOGIN REFUSED ON %s", tty);
		printf("Login incorrect\n");
		sleepexit(1);
	}

#ifdef OQUOTA
	if (quota(Q_SETUID, pwd->pw_uid, 0, 0) < 0 && errno != EINVAL) {
		switch(errno) {
		case EUSERS:
			fprintf(stderr,
		"Too many users logged on already.\nTry again later.\n");
			break;
		case EPROCLIM:
			fprintf(stderr,
			    "You have too many processes running.\n");
			break;
		default:
			perror("quota (Q_SETUID)");
		}
		sleepexit(0);
	}
#endif

	if (chdir(pwd->pw_dir) < 0) {
		printf("No directory %s!\n", pwd->pw_dir);
		if (chdir("/"))
			exit(0);
		pwd->pw_dir = "/";
		printf("Logging in with home = \"/\".\n");
	}

	/* nothing else left to fail -- really log in */
	{
		struct utmp utmp;

		memset((char *)&utmp, 0, sizeof(utmp));
		login_time = time(&utmp.ut_time);
		(void) strncpy(utmp.ut_name, username, sizeof(utmp.ut_name));
		if (hostname)
		    (void) strncpy(utmp.ut_host, hostname,
				   sizeof(utmp.ut_host));
		else
		    memset(utmp.ut_host, 0, sizeof(utmp.ut_host));
		(void) strncpy(utmp.ut_line, tty, sizeof(utmp.ut_line));
		login(&utmp);
	}

	quietlog = access(HUSHLOGIN, F_OK) == 0;
	dolastlog(quietlog, tty);

	if (!hflag && !rflag && !kflag && !Kflag && !eflag) {	/* XXX */
		static struct winsize win = { 0, 0, 0, 0 };

		(void)ioctl(0, TIOCSWINSZ, (char *)&win);
	}

	(void)chown(ttyn, pwd->pw_uid,
	    (gr = getgrnam(TTYGRPNAME)) ? gr->gr_gid : pwd->pw_gid);
#ifdef KRB4
	if(krbflag)
	    (void) chown(getenv(KRB_ENVIRON), pwd->pw_uid, pwd->pw_gid);
#endif
	(void)chmod(ttyn, 0620);
#ifdef KRB4
#ifdef SETPAG
	if (pwd->pw_uid) {
	    /* Only reset the pag for non-root users. */
	    /* This allows root to become anything. */
	    pagflag = 1;
	    setpag();
	}
#endif
	/* Fork so that we can call kdestroy */
	dofork();
#endif /* KRB4 */
	(void)setgid((gid_type) pwd->pw_gid);
	(void) initgroups(username, pwd->pw_gid);

#ifdef OQUOTA
	quota(Q_DOWARN, pwd->pw_uid, (dev_t)-1, 0);
#endif
	/* This call MUST succeed */
#ifdef _IBMR2
	setuidx(ID_LOGIN, pwd->pw_uid);
#endif
	if(setuid((uid_type) pwd->pw_uid) < 0) {
	     perror("setuid");
	     sleepexit(1);
	}

	if (*pwd->pw_shell == '\0')
		pwd->pw_shell = BSHELL;
	/* turn on new line discipline for the csh */
	else if (!strcmp(pwd->pw_shell, "/bin/csh")) {
#if !defined(_IBMR2)
		ioctlval = NTTYDISC;
		(void)ioctl(0, TIOCSETD, (char *)&ioctlval);
#endif
	}

	/* destroy environment unless user has requested preservation */
	envinit = (char **)malloc(MAXENVIRON * sizeof(char *));
	if (envinit == 0) {
		fprintf(stderr, "Can't malloc empty environment.\n");
		sleepexit(1);
	}
	if (!pflag)
		environ = envinit;

	i = 0;

#if defined(_AIX) && defined(_IBMR2)
	{
	    FILE *fp;
	    if ((fp = fopen("/etc/environment", "r")) != NULL) {
		while(fgets(tbuf, sizeof(tbuf), fp)) {
		    if ((tbuf[0] == '#') || (strchr(tbuf, '=') == 0))
			continue;
		    for (p = tbuf; *p; p++)
			if (*p == '\n') {
			    *p = '\0';
			    break;
			}
		    envinit[i++] = strsave(tbuf);
		}
		fclose(fp);
	    }
	}
#endif
	sprintf(tbuf,"LOGNAME=%s",pwd->pw_name);
	envinit[i++] = strsave(tbuf);
	sprintf(tbuf,"LOGIN=%s",pwd->pw_name);
	envinit[i++] = strsave(tbuf);

	envinit[i++] = NULL;

	setenv("HOME", pwd->pw_dir, 0);
	setenv("PATH", "/usr/local/krb5/bin:/usr/local/bin:/usr/bin/X11:/usr/ucb:/bin:/usr/bin:.", 0);
	setenv("USER", pwd->pw_name, 0);
	setenv("SHELL", pwd->pw_shell, 0);

	if (term[0] == '\0')
		(void) strncpy(term, stypeof(tty), sizeof(term));
	(void)setenv("TERM", term, 0);
#ifdef KRB4
	/* tkfile[0] is only set if we got tickets above */
	if (tkfile[0])
	    (void) setenv(KRB_ENVIRON, tkfile, 1);
#endif /* KRB4 */

#if 0
	strcpy(wgfile, "/tmp/wg.XXXXXX");
	mktemp(wgfile);
	setenv("WGFILE", wgfile, 0);
#endif

	if (tty[sizeof("tty")-1] == 'd')
		syslog(LOG_INFO, "DIALUP %s, %s", tty, pwd->pw_name);
	if (pwd->pw_uid == 0)
		if (hostname)
#ifdef KRB4
			if (kdata) {
			    /* @*$&@#*($)#@$ syslog doesn't handle very
			       many arguments */
			    char buf[BUFSIZ];
			    (void) sprintf(buf,
				   "ROOT LOGIN (krb) %s from %.*s, %s.%s@%s",
				   tty, UT_HOSTSIZE, hostname,
				   kdata->pname, kdata->pinst,
				   kdata->prealm);
			    syslog(LOG_NOTICE, buf);
		        } else {
#endif /* KRB4 */
			syslog(LOG_NOTICE, "ROOT LOGIN %s FROM %.*s",
			    tty, UT_HOSTSIZE, hostname);
#ifdef KRB4
			}
  		else 
			if (kdata) {
			    syslog(LOG_NOTICE,
				   "ROOT LOGIN (krb) %s, %s.%s@%s",
				   tty,
				   kdata->pname, kdata->pinst,
				   kdata->prealm);
			} 
#endif /* KRB4 */
		else
			syslog(LOG_NOTICE, "ROOT LOGIN %s", tty);

	if (!quietlog) {
		struct stat st;

#ifdef KRB4
		if (!krbflag)
		    printf("\nWarning: No Kerberos tickets obtained.\n\n");
#endif /* KRB4 */
		motd();
		(void)sprintf(tbuf, "%s/%s", MAILDIR, pwd->pw_name);
		if (stat(tbuf, &st) == 0 && st.st_size != 0)
			printf("You have %smail.\n",
			    (st.st_mtime > st.st_atime) ? "new " : "");
	}

#ifndef OQUOTA
	if (! access( QUOTAWARN, X_OK)) (void) system(QUOTAWARN);
#endif
	(void)signal(SIGALRM, SIG_DFL);
	(void)signal(SIGQUIT, SIG_DFL);
	(void)signal(SIGINT, SIG_DFL);
	(void)signal(SIGTSTP, SIG_IGN);

	tbuf[0] = '-';
	(void) strcpy(tbuf + 1, (p = strrchr(pwd->pw_shell, '/')) ?
	    p + 1 : pwd->pw_shell);
	execlp(pwd->pw_shell, tbuf, 0);
	fprintf(stderr, "login: no shell: ");
	perror(pwd->pw_shell);
	exit(0);
}

getloginname()
{
	register int ch;
	register char *p;
	static char nbuf[UT_NAMESIZE + 1];

	for (;;) {
		printf("login: ");
		for (p = nbuf; (ch = getchar()) != '\n'; ) {
			if (ch == EOF)
				exit(0);
			if (p < nbuf + UT_NAMESIZE)
				*p++ = ch;
		}
		if (p > nbuf)
			if (nbuf[0] == '-')
				fprintf(stderr,
				    "login names may not start with '-'.\n");
			else {
				*p = '\0';
				username = nbuf;
				break;
			}
	}
}

sigtype
timedout()
{
	fprintf(stderr, "Login timed out after %d seconds\n", timeout);
	exit(0);
}

#ifdef NOTTYENT
int root_tty_security = 1;
#endif
rootterm(tty)
	char *tty;
{
#ifdef NOTTYENT
	return(root_tty_security);
#else
	struct ttyent *t;

	return((t = getttynam(tty)) && t->ty_status&TTY_SECURE);
#endif /* NOTTYENT */
}

jmp_buf motdinterrupt;

motd()
{
	register int fd, nchars;
	sigtype (*oldint)(), sigint();
	char tbuf[8192];

	if ((fd = open(MOTDFILE, O_RDONLY, 0)) < 0)
		return;
	oldint = signal(SIGINT, sigint);
	if (setjmp(motdinterrupt) == 0)
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
	(void)signal(SIGINT, oldint);
	(void)close(fd);
}

sigtype
sigint()
{
	longjmp(motdinterrupt, 1);
}

checknologin()
{
	register int fd, nchars;
	char tbuf[8192];

	if ((fd = open(NOLOGIN, O_RDONLY, 0)) >= 0) {
		while ((nchars = read(fd, tbuf, sizeof(tbuf))) > 0)
			(void)write(fileno(stdout), tbuf, nchars);
		sleepexit(0);
	}
}

dolastlog(quiet, tty)
	int quiet;
	char *tty;
{
#if !defined(_AIX)
	struct lastlog ll;
	int fd;

	if ((fd = open(LASTLOG, O_RDWR, 0)) >= 0) {
		(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		if (!quiet) {
			if (read(fd, (char *)&ll, sizeof(ll)) == sizeof(ll) &&
			    ll.ll_time != 0) {
				printf("Last login: %.*s ",
				    24-5, (char *)ctime(&ll.ll_time));
				if (*ll.ll_host != '\0')
					printf("from %.*s\n",
					    sizeof(ll.ll_host), ll.ll_host);
				else
					printf("on %.*s\n",
					    sizeof(ll.ll_line), ll.ll_line);
			}
			(void)lseek(fd, (off_t)pwd->pw_uid * sizeof(ll), L_SET);
		}
		(void)time(&ll.ll_time);
		(void) strncpy(ll.ll_line, tty, sizeof(ll.ll_line));
		if (hostname)
		    (void) strncpy(ll.ll_host, hostname, sizeof(ll.ll_host));
		else
		    (void) memset(ll.ll_host, 0, sizeof(ll.ll_host));
		(void)write(fd, (char *)&ll, sizeof(ll));
		(void)close(fd);
	}
#endif
}

#undef	UNKNOWN
#define	UNKNOWN	"su"

char *
stypeof(ttyid)
	char *ttyid;
{
#ifdef NOTTYENT
	return(UNKNOWN);
#else
	struct ttyent *t;

	return(ttyid && (t = getttynam(ttyid)) ? t->ty_type : UNKNOWN);
#endif
}

doremotelogin(host)
	char *host;
{
	static char lusername[UT_NAMESIZE+1];
	char rusername[UT_NAMESIZE+1];

	lgetstr(rusername, sizeof(rusername), "Remote user");
	lgetstr(lusername, sizeof(lusername), "Local user");
	lgetstr(term, sizeof(term), "Terminal type");
	username = lusername;
	pwd = getpwnam(username);
	if (pwd == NULL)
		return(-1);
	return(ruserok(host, (pwd->pw_uid == 0), rusername, username));
}

#ifdef KRB4
do_krb_login(host, strict)
	char *host;
	int strict;
{
	int rc;
	struct sockaddr_in sin;
	char instance[INST_SZ], version[9];
	long authoptions = 0L;
        struct hostent *hp = gethostbyname(host);
	static char lusername[UT_NAMESIZE+1];

	/*
	 * Kerberos autologin protocol.
	 */

	(void) memset((char *) &sin, 0, (int) sizeof(sin));

        if (hp)
                (void) bcopy (hp->h_addr, (char *)&sin.sin_addr,
			      sizeof(sin.sin_addr));
	else
		sin.sin_addr.s_addr = inet_addr(host);

	if ((hp == NULL) && (sin.sin_addr.s_addr == -1)) {
	    printf("Hostname did not resolve to an address, so Kerberos authentication failed\r\n");
                /*
		 * No host addr prevents auth, so
                 * punt krb and require password
		 */
                if (strict) {
                        goto paranoid;
                } else {
			pwd = NULL;
                        return(-1);
		}
	}

	kdata = (AUTH_DAT *)malloc( sizeof(AUTH_DAT) );
	ticket = (KTEXT) malloc(sizeof(KTEXT_ST));

	(void) strcpy(instance, "*");
	if (rc=krb_recvauth(authoptions, 0, ticket, "rcmd",
			    instance, &sin,
			    (struct sockaddr_in *)0,
			    kdata, "", (bit_64 *) 0, version)) {
		printf("Kerberos rlogin failed: %s\r\n",krb_err_txt[rc]);
		if (strict) {
paranoid:
			/*
			 * Paranoid hosts, such as a Kerberos server,
			 * specify the Klogind daemon to disallow
			 * even password access here.
			 */
			printf("Sorry, you must have Kerberos authentication to access this host.\r\n");
			exit(1);
		}
	}
	(void) lgetstr(lusername, sizeof (lusername), "Local user");
	(void) lgetstr(term, sizeof(term), "Terminal type");
	username = lusername;
	if (getuid()) {
		pwd = NULL;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = NULL;
		return(-1);
	}

	/*
	 * if Kerberos login failed because of an error in krb_recvauth,
	 * return the indication of a bad attempt.  User will be prompted
	 * for a password.  We CAN'T check the .rhost file, because we need 
	 * the remote username to do that, and the remote username is in the 
	 * Kerberos ticket.  This affects ONLY the case where there is
	 * Kerberos on both ends, but Kerberos fails on the server end. 
	 */
	if (rc) {
		return(-1);
	}

	if (rc=kuserok(kdata,lusername)) {
		printf("login: %s has not given you permission to login without a password.\r\n",lusername);
		if (strict) {
		  exit(1);
		}
		return(-1);
	}
	return(0);
}
#endif /* KRB4 */

lgetstr(buf, cnt, err)
	char *buf, *err;
	int cnt;
{
	int ocnt = cnt;
	char *obuf = buf;
	char ch;

	do {
		if (read(0, &ch, sizeof(ch)) != sizeof(ch))
			exit(1);
		if (--cnt < 0) {
			fprintf(stderr,"%s '%.*s' too long, %d characters maximum.\r\n",
			       err, ocnt, obuf, ocnt-1);
			sleepexit(1);
		}
		*buf++ = ch;
	} while (ch);
}

char *speeds[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600",
	"1200", "1800", "2400", "4800", "9600", "19200", "38400",
};
#define	NSPEEDS	(sizeof(speeds) / sizeof(speeds[0]))

doremoteterm(tp)
#ifdef POSIX
	struct termios *tp;
#else
	struct sgttyb *tp;
#endif
{
	register char *cp = strchr(term, '/'), **cpp;
	char *speed;

	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = strchr(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
#ifdef POSIX
				tp->c_cflag =
					(tp->c_cflag & ~CBAUD) | (cpp-speeds);
#else
				tp->sg_ispeed = tp->sg_ospeed = cpp-speeds;
#endif
				break;
			}
	}
#ifdef POSIX
 	/* set all standard echo, edit, and job control options */
 	tp->c_lflag = ECHO|ECHOE|ECHOK|ICANON|ISIG;
 	tp->c_iflag |= ICRNL|BRKINT;
 	tp->c_oflag |= ONLCR|OPOST|TAB3;
#else /* !POSIX */
	tp->sg_flags = ECHO|CRMOD|ANYP|XTABS;
#endif
}

sleepexit(eval)
	int eval;
{
#ifdef KRB4
	if (krbflag)
	    (void) dest_tkt();
#endif /* KRB4 */
	sleep((u_int)5);
	exit(eval);
}

#ifdef KRB4
/*
 * This routine handles cleanup stuff, and the like.
 * It exits only in the child process.
 */
#include <sys/wait.h>
void
dofork()
{
    int child;

#ifdef _IBMR2
    update_ref_count(1);
#endif
    if(!(child=fork()))
	    return; /* Child process */

    /* Setup stuff?  This would be things we could do in parallel with login */
    (void) chdir("/");	/* Let's not keep the fs busy... */
    
    /* If we're the parent, watch the child until it dies */
    while(wait((union wait *)0) != child)
	    ;

    /* Cleanup stuff */
    /* Run dest_tkt to destroy tickets */
    (void) dest_tkt();		/* If this fails, we lose quietly */
#ifdef SETPAG
    if (pagflag)
	ktc_ForgetAllTokens();
#endif
#ifdef _IBMR2
    update_ref_count(-1);
#endif

    /* Leave */
    exit(0);
}
#endif /* KRB4 */


char *strsave(s)
char *s;
{
    char *ret = (char *)malloc(strlen(s) + 1);
    strcpy(ret, s);
    return(ret);
}

#ifdef _IBMR2
update_ref_count(int adj)
{
    static char *empty = "\0";
    char *grp;
    int i;

    /* Update reference count on all user's temporary groups */
    setuserdb(S_READ|S_WRITE);
    if (getuserattr(username, S_GROUPS, (void *)&grp, SEC_LIST) == 0) {
	while (*grp) {
	    if (getgroupattr(grp, "athena_temp", (void *)&i, SEC_INT) == 0) {
		i += adj;
		if (i > 0) {
		    putgroupattr(grp, "athena_temp", (void *)i, SEC_INT);
		    putgroupattr(grp, (char *)0, (void *)0, SEC_COMMIT);
		} else {
		    putgroupattr(grp, S_USERS, (void *)empty, SEC_LIST);
		    putgroupattr(grp, (char *)0, (void *)0, SEC_COMMIT);
		    rmufile(grp, 0, GROUP_TABLE);
		}
	    }
	    while (*grp) grp++;
	    grp++;
	}
    }
    enduserdb();
}
#endif
