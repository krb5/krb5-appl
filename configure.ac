AC_INIT(Kerberos 5 applications)
AC_CONFIG_SRCDIR(k5-platform.h)
AC_CONFIG_HEADERS(autoconf.h)

# Find the krb5 library installation.
AC_ARG_WITH(krb5, [AS_HELP_STRING([--with-krb5=path],[where krb5 resides])])
if test "$with_krb5" = no; then
    AC_MSG_ERROR([This package requires krb5])
elif test -z "$with_krb5" -o "$with_krb5" = yes; then
    krb5_config=krb5-config
elif test -x "$with_krb5/bin/krb5-config"; then
    # Assume krb5 exec prefix specified.
    krb5_config=$with_krb5/bin/krb5-config
elif test -f "$with_krb5" -a -x "$with_krb5"; then
    # Assume path to krb5-config specified.
    krb5_config=$with_krb5
fi
if test -z "$krb5_config" || test -z "`$krb5_config --prefix`"; then
  AC_MSG_ERROR([Specify -with-krb5= krb5 exec-prefix or path to krb5-config])
fi
KRB5_CFLAGS=`$krb5_config --cflags`
KRB5_BASE_LIBS=`$krb5_config --libs krb5`
GSS_LIBS=`$krb5_config --libs gssapi`
KRB5_BINDIR=`$krb5_config --exec-prefix`/bin
KRB5_SBINDIR=`$krb5_config --exec-prefix`/sbin
AC_SUBST(KRB5_CFLAGS)
AC_SUBST(KRB5_BASE_LIBS)
AC_SUBST(GSS_LIBS)
AC_SUBST(KRB5_BINDIR)
AC_SUBST(KRB5_SBINDIR)

WITH_CC
if test -z "$LD" ; then LD=$CC; fi
AC_ARG_VAR(LD,[linker command [CC]])
AC_SUBST(LDFLAGS)
AC_C_CONST
AC_ARG_PROGRAM
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_ARCHIVE
AC_PROG_ARCHIVE_ADD
AC_PROG_INSTALL
AC_PROG_YACC
AC_CHECK_PROG(AR, ar, ar, false)
AC_FUNC_FORK
AC_TYPE_MODE_T
AC_TYPE_SIGNAL
AC_CHECK_TYPES([socklen_t, struct sockaddr_storage],,,
[#include <sys/types.h>
#include <sys/socket.h>
])

# Enables system protoypes for extended functions (such as asprintf)
# on most Linux systems.
AC_DEFINE(_GNU_SOURCE, 1, [Enable extended glibc prototypes])

AC_HEADER_STDARG
AC_HEADER_TIME
CHECK_DIRENT
CHECK_SETJMP
CHECK_SIGNALS
CHECK_SIGPROCMASK
CHECK_UTMP
CHECK_WAIT_TYPE
DECLARE_SYS_ERRLIST
KRB5_AC_MAINTAINER_MODE
KRB5_AC_INET6
KRB5_AC_LIBUTIL
KRB5_SIGTYPE
KRB5_GETSOCKNAME_ARGS

AC_CHECK_HEADERS(curses.h lastlog.h libutil.h memory.h paths.h pty.h stdlib.h)
AC_CHECK_HEADERS(string.h ttyent.h util.h sac.h unistd.h utmp.h utmpx.h)
AC_CHECK_HEADERS(arpa/inet.h arpa/nameser.h)
AC_CHECK_HEADERS(sys/filio.h sys/ioctl_compat.h sys/label.h sys/param.h)
AC_CHECK_HEADERS(sys/ptyvar.h sys/select.h sys/sockio.h sys/stream.h)
AC_CHECK_HEADERS(sys/time.h sys/tty.h sys/uio.h sys/utsname.h sys/wait.h)
# On some systems, term.h requires curses.h inclusion
AC_CHECK_HEADERS(term.h,,,
[#ifdef HAVE_CURSES_H
#include <curses.h>
#endif
])

case $krb5_cv_host in
*-*-solaris*)
    if test "$ac_cv_c_compiler_gnu" = yes; then
        # Solaris 8 at least has curses.h that is noisy under gcc
        ac_cv_header_curses_h=yes
    fi
    ;;
esac

AC_CHECK_FUNCS(_getpty cgetent getcwd getenv gettosbyname getusershell getutmp)
AC_CHECK_FUNCS(getutmpx grantpt inet_aton initgroups isatty killpg killpg)
AC_CHECK_FUNCS(line_push logwtmp openpty ptsname revoke rmufile rresvport_af)
AC_CHECK_FUNCS(seteuid setlogin setpgid setpriority setresuid setreuid)
AC_CHECK_FUNCS(setutent setutsent setutxent strsave tcgetpgrp tcsetpgrp)
AC_CHECK_FUNCS(ttyname unsetenv updwtmp updwtmpx utimes utmpname utmpxname)
AC_CHECK_FUNCS(vhangup vsnprintf waitpid)

# Determine which functions to define in libmissing.
AC_REPLACE_FUNCS(daemon getdtablesize getopt herror parsetos setenv setsid)
AC_REPLACE_FUNCS(strcasecmp strdup strerror strftime strlcpy asprintf)

# Determine libraries for login and ftpd.
LOGINLIBS=
FTPD_LIBS=
AC_ARG_WITH([afs],
[  --without-afs        don't have afs libraries to build against (default)
  --with-afs=AFSDIR    use preinstalled AFS library tree],
,with_afs=no)
if test $with_afs != no; then
    AC_DEFINE(SETPAG,1,[Define if setpag should be used])
    LOGINLIBS="$LOGINLIBS -L$with_afs/lib -L$with_afs/lib/afs"
    LOGINLIBS="$LOGINLIBS -lauth -lsys -lrx -llwp"
fi
AC_CHECK_LIB(crypt,crypt,
             [LOGINLIBS="$LOGINLIBS -lcrypt"
              FTPD_LIBS="$FTPD_LIBS -lcrypt"])
# AIX has all three of these; SCO might too.
AC_CHECK_LIB(odm,main,
    AC_CHECK_LIB(s,main,
        AC_CHECK_LIB(cfg,main, 
        [LOGINLIBS="$LOGINLIBS -lodm -ls -lcfg"])))
KRSHDLIBS="$LOGINLIBS"
AC_SUBST(KRSHDLIBS)
AC_SUBST(LOGINLIBS)
AC_SUBST(FTPD_LIBS)

# Determine libraries for telnet and telnetd.
old_LIBS="$LIBS"
AC_CHECK_LIB(termcap,main,
             [AC_DEFINE(TERMCAP,1,[Define if termcap library is available])
              LIBS="$LIBS -ltermcap"])
AC_CHECK_LIB(curses,setupterm,LIBS="$LIBS -lcurses",
             [AC_CHECK_LIB(ncurses,setupterm,LIBS="$LIBS -lncurses")])
AC_CHECK_FUNCS(setupterm)
TELNET_LIBS="$LIBS"
TELNETD_LIBS="$LIBS"
AC_SUBST(TELNET_LIBS)
AC_SUBST(TELNETD_LIBS)
AC_CHECK_FUNC(tgetent, ,
              [AC_MSG_ERROR([Could not find tgetent; are you missing a curses/ncurses library?])])
LIBS="$old_LIBS"

# Make our operating system-specific security checks and definitions
# for libpty, login, and ftpd.  The following code decides what
# streams modules will be pushed onto a pty.  In particular, if
# HAVE_STREAMS is defined and HAVE_LINE_PUSH is not defined, modules
# may be pushed by inserting an appropriate generic ifdef for each
# module in init_slave.c and AC_DEFINES for the operating systems that
# need the modules.  Each OS that supports streams has a different
# idea of what you want to push.
case $krb5_cv_host in
*-*-ultrix*)
    echo "Disabling initial vhangup and setsid because they break under Ultrix"
    AC_DEFINE([OPEN_CTTY_ONLY_ONCE],[1],
              [Define on Ultrix where an initial vhangup breaks])
    ac_cv_func_setsid=no # setsid doesn't do the right thing under Ultrix
    ;;
*-*-aix3*)
    # AIX has streams include files but not streams TTY
    # Moreover, strops.h trashes sys/ioctl.h
    krb5_cv_has_streams=no
    ;;
alpha*-dec-osf*)
    AC_MSG_RESULT(will open ctty prior to revoke due to OSF/1 lossage)
    AC_DEFINE(REVOKE_NEEDS_OPEN,1,
              [Define if ctty needs to be opened before revoke as on OSF/1])
    AC_CHECK_LIB(security,setluid,
                 AC_DEFINE(HAVE_SETLUID,1,
                 [Define if setluid is supplied by the OSF/1 security library])
                 LOGINLIBS="$LOGINLIBS -lsecurity"
                 FTPD_LIBS="$FTPD_LIBS -lsecurity")
    ;;
*-*-solaris*)
    AC_DEFINE(PUSH_PTEM,1,[push ptem?])
    AC_DEFINE(PUSH_LDTERM,1,[push ldterm?])
    AC_DEFINE(PUSH_TTCOMPAT,1,[push ttcompat?])
    ;;
*-*-hpux*|*-*-linux*|*-*-irix*)
    krb5_cv_has_streams=no
    ;;
esac

AC_MSG_CHECKING([streams interface])
AC_CACHE_VAL(krb5_cv_has_streams,
[AC_TRY_COMPILE(
[#include <sys/stream.h>
#include <sys/stropts.h>], [],
krb5_cv_has_streams=yes, krb5_cv_has_streams=no)])
AC_MSG_RESULT($krb5_cv_has_streams)
if test $krb5_cv_has_streams = yes; then
AC_DEFINE(HAVE_STREAMS,1,[Define if have streams])
fi

# telnet checks for STREAMSPTY to indicate streams and grantpt.
if test "$krb5_cv_has_streams" = yes -a "$ac_cv_func_grantpt" = yes; then
    AC_DEFINE(STREAMSPTY,1,[Define if streams pty interface should be used])
fi

# ----- utmp stuff -----

AC_DEFUN(K5_CHECK_UT_MEMBER,
[AC_MSG_CHECKING([for $2 in struct $1])
AC_CACHE_VAL([krb5_cv_struct_$1_$2],
[AC_TRY_COMPILE([#include <sys/types.h>
#include <$1.h>], [struct $1 u; u.$2;],
eval "krb5_cv_struct_$1_$2=yes", eval "krb5_cv_struct_$1_$2=no")])
if eval "test \"`echo '$krb5_cv_struct_'$1'_'$2`\" = yes"; then
  AC_MSG_RESULT(yes)
  krb5_tr_ut=HAVE_STRUCT_`echo $1'_'$2 | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  AC_DEFINE_UNQUOTED($krb5_tr_ut,1,[Define if $2 field present in $1])
else
  AC_MSG_RESULT(no)
fi])

if test "$ac_cv_header_utmp_h" = yes; then
  AC_MSG_RESULT(checking struct utmp members)
  for krb5_mem in ut_host ut_syslen ut_addr ut_id ut_pid ut_type ut_exit; do
    K5_CHECK_UT_MEMBER(utmp, $krb5_mem)
  done
fi

if test "$ac_cv_header_utmpx_h" = yes; then
  AC_MSG_RESULT(checking struct utmpx members)
  for krb5_mem in ut_host ut_syslen ut_addr ut_id ut_pid ut_type ut_exit; do
    K5_CHECK_UT_MEMBER(utmpx, $krb5_mem)
  done
fi

AC_DEFUN(K5_CHECK_UT_EXIT_MEMBER,
[AC_MSG_CHECKING([for ut_exit.$2 in struct $1])
AC_CACHE_VAL([krb5_cv_struct_$1_ut_exit_$2],
[AC_TRY_COMPILE([#include <sys/types.h>
#include <$1.h>], [struct $1 u; u.ut_exit.$2;],
eval "krb5_cv_struct_$1_ut_exit_$2=yes",
eval "krb5_cv_struct_$1_ut_exit_$2=no")])
if eval "test \"`echo '$krb5_cv_struct_'$1'_ut_exit_'$2`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], , :, [$3])
else
  AC_MSG_RESULT(no)
  ifelse([$4], , :, [$4])
fi])

if test "$krb5_cv_struct_utmp_ut_exit" = yes; then
  AC_MSG_RESULT(checking for working ut_exit.e_exit in struct utmp)
  for krb5_mem in __e_exit ut_e_exit ut_exit e_exit; do
    K5_CHECK_UT_EXIT_MEMBER(utmp, $krb5_mem,
[krb5_utmp_e_exit=$krb5_mem
krb5_utmp_e_termination=`echo $krb5_mem|sed -e 's%_exit$%_termination%'`], )
  done
  if test "${krb5_utmp_e_exit+set}" = set; then
    AC_MSG_RESULT([working ut_exit.e_exit in utmp is $krb5_utmp_e_exit])
    AC_DEFINE_UNQUOTED(PTY_UTMP_E_EXIT, $krb5_utmp_e_exit,[Define to utmp exit field name])
    AC_DEFINE_UNQUOTED(PTY_UTMP_E_TERMINATION, $krb5_utmp_e_termination,[Define to utmp termination field name])
  else
    AC_MSG_RESULT([cannot find working ut_exit.e_exit in utmp])
  fi
fi

if test "$krb5_cv_struct_utmpx_ut_exit" = yes; then
  AC_MSG_RESULT(checking for working ut_exit.e_exit in struct utmpx)
  for krb5_mem in __e_exit ut_e_exit ut_exit e_exit; do
    K5_CHECK_UT_EXIT_MEMBER(utmpx, $krb5_mem,
[krb5_utmpx_e_exit=$krb5_mem
krb5_utmpx_e_termination=`echo $krb5_mem|sed -e 's%_exit$%_termination%'`], )
  done
  if test "${krb5_utmpx_e_exit+set}" = set; then
    AC_MSG_RESULT([working ut_exit.e_exit in utmpx is $krb5_utmpx_e_exit])
    AC_DEFINE_UNQUOTED(PTY_UTMPX_E_EXIT, $krb5_utmpx_e_exit,[Define to utmpx exit field name])
    AC_DEFINE_UNQUOTED(PTY_UTMPX_E_TERMINATION, $krb5_utmpx_e_termination,[Define to utmpx termination field name])
  else
    AC_MSG_RESULT([cannot find working ut_exit.e_exit in utmpx])
  fi
fi

if test "$ac_cv_header_utmpx_h" = yes; then
  AC_MSG_CHECKING(consistency of utmpx API)
  if test "$ac_cv_func_setutxent" = yes; then
    if test "$krb5_cv_struct_utmpx_ut_id" = yes \
      && test "$krb5_cv_struct_utmpx_ut_type" = yes \
      && test "$krb5_cv_struct_utmpx_ut_pid" = yes; then
      AC_MSG_RESULT(ok)
    else
      AC_MSG_RESULT(not ok)
      AC_MSG_ERROR([have setutxent but no ut_id, ut_type, or ut_pid in utmpx])
    fi
  else
    AC_MSG_RESULT(not ok)
    AC_MSG_ERROR([have utmpx.h but no setutxent])
  fi
fi

if test "$ac_cv_func_setutent" = yes && \
  test "$ac_cv_header_utmpx_h" = no; then
  AC_MSG_CHECKING(consistency of sysV-ish utmp API)
  if test "$ac_cv_header_utmp_h" = yes; then
    if test "$krb5_cv_struct_utmp_ut_id" = yes \
      && test "$krb5_cv_struct_utmp_ut_type" = yes \
      && test "$krb5_cv_struct_utmp_ut_pid" = yes; then
      AC_MSG_RESULT(ok)
    else
      AC_MSG_RESULT(not ok)
      AC_MSG_ERROR([have setutent but no ut_id, ut_type, or ut_pid in utmp])
    fi
  else
    AC_MSG_RESULT(not ok)
    AC_MSG_ERROR([have setutent but no utmp.h])
  fi
fi

# ----- End of utmp stuff -----

AC_MSG_CHECKING([arguments to getpgrp])
AC_CACHE_VAL(krb5_cv_getpgrp_args,
[AC_TRY_COMPILE(
[#ifndef __STDC__
#define __STDC__ 1
#endif
#include <unistd.h>
#include <sys/types.h>], [pid_t pid = getpgrp(getpid())],
krb5_cv_getpgrp_args=pid, krb5_cv_getpgrp_args=void)])
AC_MSG_RESULT($krb5_cv_getpgrp_args)
if test $krb5_cv_getpgrp_args = pid; then
AC_DEFINE(GETPGRP_ONEARG,1,[Define if getpgrp takes one arg])
fi

AC_MSG_CHECKING([if setpgrp takes two arguments])
AC_CACHE_VAL(krb5_cv_sys_setpgrp_two,
[AC_TRY_COMPILE( 
[#include <unistd.h>],[setpgrp(0,0)],
krb5_cv_sys_setpgrp_two=yes,krb5_cv_sys_setpgrp_two=no)])
AC_MSG_RESULT($krb5_cv_sys_setpgrp_two)
if test $krb5_cv_sys_setpgrp_two = yes; then
  AC_DEFINE(SETPGRP_TWOARG,1,[Define if setpgrp takes two arguments])
fi

AC_MSG_CHECKING([F_SETOWN])
AC_CACHE_VAL(krb5_cv_f_setown,
[AC_TRY_COMPILE(
[#include <sys/types.h>
#include <fcntl.h>], [1+F_SETOWN;],
krb5_cv_f_setown=yes,krb5_cv_f_setown=no)])
AC_MSG_RESULT($krb5_cv_f_setown)
if test $krb5_cv_f_setown = yes; then
AC_DEFINE(HAVE_SETOWN,1,[Define if F_SETOWN is available])
fi

AC_MSG_CHECKING([shadow password support])
AC_CACHE_VAL(krb5_cv_shadow_pwd,
[AC_TRY_LINK(
[#include <sys/types.h>
#include <pwd.h>
#include <shadow.h>],
[struct spwd *sp = getspnam("root")],
krb5_cv_shadow_pwd=yes, krb5_cv_shadow_pwd=no)])
AC_MSG_RESULT($krb5_cv_shadow_pwd)
if test $krb5_cv_shadow_pwd = yes; then
AC_DEFINE(HAVE_SHADOW,1,[Define if shadow password interface is available])
fi
AC_CHECK_FILES(/etc/environment /etc/TIMEZONE)

# bsd and libpty check POSIX_TERMIOS; telnet checks USE_TERMIO.
AC_CHECK_HEADER(termios.h,
    [AC_CHECK_FUNC(cfsetispeed,
        [AC_DEFINE(POSIX_TERMIOS,1,[Define for POSIX termios interface])
         AC_DEFINE(USE_TERMIO,1,[Define if termio should be used])
         ac_termio=1])])
if test -z "$ac_termio"; then
    AC_CHECK_HEADER(termio.h,
                    [AC_DEFINE(SYSV_TERMIO,1,
                               [Define if SysV termio interface is found])
                     ac_sysv_termio=1])
    if test -n "$ac_sysv_termio"; then
        AC_MSG_CHECKING([for cc_t in termio.h])
        AC_CACHE_VAL(krb_cv_type_cc_t,
                     [AC_TRY_LINK([cc_t],[#include <termio.h>],[cc_t foo;],
                                  [krb_cv_type_cc_t=yes],
                                  [krb_cv_type_cc_t=no])])
        AC_MSG_RESULT($krb_cv_type_cc_t)
        if test $krb_cv_type_cc_t = no; then
            AC_DEFINE(NO_CC_T,1,[Define if termio.h does not define type cc_t])
        fi
    fi
fi

AC_CACHE_CHECK([if speed_t is defined], krb5_cv_type_speed_t,
[AC_TRY_COMPILE(dnl
[#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#ifndef	USE_TERMIO
#include <sgtty.h>
#else
# ifdef	SYSV_TERMIO
# include <termio.h>
# else
# include <termios.h>
# endif
#endif
],[speed_t termspeed],krb5_cv_type_speed_t=yes, krb5_cv_type_speed_t=no)])
if test $krb5_cv_type_speed_t = no; then
   AC_DEFINE(speed_t, int,
             [Define if system termios interface doesn't define speed_t])
fi

# For dejagnu tests
AC_CHECK_PROG(RUNTEST,runtest,runtest)
if test x"$RUNTEST" != x; then
    HAVE_RUNTEST=yes
else
    HAVE_RUNTEST=no
fi
AC_SUBST(HAVE_RUNTEST)

V5_AC_OUTPUT_MAKEFILE(. bsd libmissing libpty
                      gssftp gssftp/ftp gssftp/ftpd
                      telnet telnet/libtelnet telnet/telnet telnet/telnetd
                      tests tests/resolve)
