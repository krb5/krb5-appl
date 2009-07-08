AC_PREREQ(2.52)
AC_COPYRIGHT([Copyright 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2008, 2009
Massachusetts Institute of Technology.
])dnl
dnl Maintainer mode, akin to what automake provides, 'cept we don't
dnl want to use automake right now.
AC_DEFUN([KRB5_AC_MAINTAINER_MODE],
[AC_ARG_ENABLE([maintainer-mode],
AC_HELP_STRING([--enable-maintainer-mode],[enable rebuilding of source files, Makefiles, etc]),
USE_MAINTAINER_MODE=$enableval,
USE_MAINTAINER_MODE=no)
if test "$USE_MAINTAINER_MODE" = yes; then
  MAINTAINER_MODE_TRUE=
  MAINTAINER_MODE_FALSE='#'
  AC_MSG_NOTICE(enabling maintainer mode)
else
  MAINTAINER_MODE_TRUE='#'
  MAINTAINER_MODE_FALSE=
fi
MAINT=$MAINTAINER_MODE_TRUE
AC_SUBST(MAINTAINER_MODE_TRUE)
AC_SUBST(MAINTAINER_MODE_FALSE)
AC_SUBST(MAINT)
])dnl
dnl
dnl DECLARE_SYS_ERRLIST - check for sys_errlist in libc
dnl
AC_DEFUN([DECLARE_SYS_ERRLIST],
[AC_CACHE_CHECK([for sys_errlist declaration], krb5_cv_decl_sys_errlist,
[AC_TRY_COMPILE([#include <stdio.h>
#include <errno.h>], [1+sys_nerr;],
krb5_cv_decl_sys_errlist=yes, krb5_cv_decl_sys_errlist=no)])
# assume sys_nerr won't be declared w/o being in libc
if test $krb5_cv_decl_sys_errlist = yes; then
  AC_DEFINE(SYS_ERRLIST_DECLARED,1,[Define if sys_errlist is defined in errno.h])
  AC_DEFINE(HAVE_SYS_ERRLIST,1,[Define if sys_errlist in libc])
else
  # This means that sys_errlist is not declared in errno.h, but may still
  # be in libc.
  AC_CACHE_CHECK([for sys_errlist in libc], krb5_cv_var_sys_errlist,
  [AC_TRY_LINK([extern int sys_nerr;], [if (1+sys_nerr < 0) return 1;],
  krb5_cv_var_sys_errlist=yes, krb5_cv_var_sys_errlist=no;)])
  if test $krb5_cv_var_sys_errlist = yes; then
    AC_DEFINE(HAVE_SYS_ERRLIST,1,[Define if sys_errlist in libc])
    # Do this cruft for backwards compatibility for now.
    AC_DEFINE(NEED_SYS_ERRLIST,1,[Define if need to declare sys_errlist])
  else
    AC_MSG_WARN([sys_errlist is neither in errno.h nor in libc])
  fi
fi])dnl
dnl
dnl check for sigmask/sigprocmask -- CHECK_SIGPROCMASK
dnl
AC_DEFUN(CHECK_SIGPROCMASK,[
AC_MSG_CHECKING([for use of sigprocmask])
AC_CACHE_VAL(krb5_cv_func_sigprocmask_use,
[AC_TRY_LINK([#include <signal.h>], [sigprocmask(SIG_SETMASK,0,0);],
 krb5_cv_func_sigprocmask_use=yes,
AC_TRY_LINK([#include <signal.h>], [sigmask(1);], 
 krb5_cv_func_sigprocmask_use=no, krb5_cv_func_sigprocmask_use=yes))])
AC_MSG_RESULT($krb5_cv_func_sigprocmask_use)
if test $krb5_cv_func_sigprocmask_use = yes; then
 AC_DEFINE(USE_SIGPROCMASK,1,[Define if sigprocmask should be used])
fi
])dnl
dnl
AC_DEFUN(AC_PROG_ARCHIVE, [AC_CHECK_PROG(ARCHIVE, ar, ar cqv, false)])dnl
AC_DEFUN(AC_PROG_ARCHIVE_ADD, [AC_CHECK_PROG(ARADD, ar, ar cruv, false)])dnl
dnl
dnl check for <dirent.h> -- CHECK_DIRENT
dnl (may need to be more complex later)
dnl
AC_DEFUN(CHECK_DIRENT,[
AC_CHECK_HEADER(dirent.h,AC_DEFINE(USE_DIRENT_H,1,[Define if you have dirent.h functionality]))])dnl
dnl
dnl check if union wait is defined, or if WAIT_USES_INT -- CHECK_WAIT_TYPE
dnl
AC_DEFUN(CHECK_WAIT_TYPE,[
AC_MSG_CHECKING([if argument to wait is int *])
AC_CACHE_VAL(krb5_cv_struct_wait,
dnl Test for prototype clash - if there is none - then assume int * works
[AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/wait.h>
extern pid_t wait(int *);],[], krb5_cv_struct_wait=no,dnl
dnl Else fallback on old stuff
[AC_TRY_COMPILE(
[#include <sys/wait.h>], [union wait i;
#ifdef WEXITSTATUS
  WEXITSTATUS (i);
#endif
], 
	krb5_cv_struct_wait=yes, krb5_cv_struct_wait=no)])])
AC_MSG_RESULT($krb5_cv_struct_wait)
if test $krb5_cv_struct_wait = no; then
	AC_DEFINE(WAIT_USES_INT,1,[Define if wait takes int as a argument])
fi
])dnl
dnl
dnl check for POSIX signal handling -- CHECK_SIGNALS
dnl
AC_DEFUN(CHECK_SIGNALS,[
AC_CHECK_FUNC(sigprocmask,
AC_MSG_CHECKING(for sigset_t and POSIX_SIGNALS)
AC_CACHE_VAL(krb5_cv_type_sigset_t,
[AC_TRY_COMPILE(
[#include <signal.h>],
[sigset_t x],
krb5_cv_type_sigset_t=yes, krb5_cv_type_sigset_t=no)])
AC_MSG_RESULT($krb5_cv_type_sigset_t)
if test $krb5_cv_type_sigset_t = yes; then
  AC_DEFINE(POSIX_SIGNALS,1,[Define if POSIX signal handling is used])
fi
)])dnl
dnl
dnl check for signal type
dnl
dnl AC_RETSIGTYPE isn't quite right, but almost.
AC_DEFUN(KRB5_SIGTYPE,[
AC_MSG_CHECKING([POSIX signal handlers])
AC_CACHE_VAL(krb5_cv_has_posix_signals,
[AC_TRY_COMPILE(
[#include <sys/types.h>
#include <signal.h>
#ifdef signal
#undef signal
#endif
extern void (*signal ()) ();], [],
krb5_cv_has_posix_signals=yes, krb5_cv_has_posix_signals=no)])
AC_MSG_RESULT($krb5_cv_has_posix_signals)
if test $krb5_cv_has_posix_signals = yes; then
   stype=void
   AC_DEFINE(POSIX_SIGTYPE, 1, [Define if POSIX signal handlers are used])
else
  if test $ac_cv_type_signal = void; then
     stype=void
  else
     stype=int
  fi
fi
AC_DEFINE_UNQUOTED(krb5_sigtype, $stype, [Define krb5_sigtype to type of signal handler])dnl
])dnl
dnl
dnl check for POSIX setjmp/longjmp -- CHECK_SETJMP
dnl
AC_DEFUN(CHECK_SETJMP,[
AC_CHECK_FUNC(sigsetjmp,
AC_MSG_CHECKING(for sigjmp_buf)
AC_CACHE_VAL(krb5_cv_struct_sigjmp_buf,
[AC_TRY_COMPILE(
[#include <setjmp.h>],[sigjmp_buf x],
krb5_cv_struct_sigjmp_buf=yes,krb5_cv_struct_sigjmp_buf=no)])
AC_MSG_RESULT($krb5_cv_struct_sigjmp_buf)
if test $krb5_cv_struct_sigjmp_buf = yes; then
  AC_DEFINE(POSIX_SETJMP,1,[Define if setjmp indicates POSIX interface])
fi
)])dnl
dnl
dnl Check for IPv6 compile-time support.
dnl
AC_DEFUN(KRB5_AC_INET6,[
AC_CHECK_HEADERS(sys/types.h sys/socket.h netinet/in.h netdb.h)
AC_CHECK_FUNCS(inet_ntop inet_pton getnameinfo)
dnl getaddrinfo test needs netdb.h, for proper compilation on alpha
dnl under OSF/1^H^H^H^H^HDigital^H^H^H^H^H^H^HTru64 UNIX, where it's
dnl a macro
AC_MSG_CHECKING(for getaddrinfo)
AC_CACHE_VAL(ac_cv_func_getaddrinfo,
[AC_TRY_LINK([#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif],[
struct addrinfo *ai;
getaddrinfo("kerberos.mit.edu", "echo", 0, &ai);
], ac_cv_func_getaddrinfo=yes, ac_cv_func_getaddrinfo=no)])
AC_MSG_RESULT($ac_cv_func_getaddrinfo)
if test $ac_cv_func_getaddrinfo = yes; then
  AC_DEFINE(HAVE_GETADDRINFO,1,[Define if you have the getaddrinfo function])
fi
dnl
AC_REQUIRE([KRB5_SOCKADDR_SA_LEN])dnl
AC_ARG_ENABLE([ipv6], , AC_MSG_WARN(enable/disable-ipv6 option is deprecated))dnl
AC_MSG_CHECKING(for IPv6 compile-time support)
AC_CACHE_VAL(krb5_cv_inet6,[
if test "$ac_cv_func_inet_ntop" != "yes" ; then
  krb5_cv_inet6=no
else
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
],[
  struct sockaddr_in6 in;
  AF_INET6;
  IN6_IS_ADDR_LINKLOCAL (&in.sin6_addr);
],krb5_cv_inet6=yes,krb5_cv_inet6=no)])
fi
AC_MSG_RESULT($krb5_cv_inet6)
if test "$krb5_cv_inet6" = no && test "$ac_cv_func_inet_ntop" = yes; then
AC_MSG_CHECKING(for IPv6 compile-time support with -DINET6)
AC_CACHE_VAL(krb5_cv_inet6_with_dinet6,[
old_CC="$CC"
CC="$CC -DINET6"
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
],[
  struct sockaddr_in6 in;
  AF_INET6;
  IN6_IS_ADDR_LINKLOCAL (&in.sin6_addr);
],krb5_cv_inet6_with_dinet6=yes,krb5_cv_inet6_with_dinet6=no)
CC="$old_CC"])
AC_MSG_RESULT($krb5_cv_inet6_with_dinet6)
fi
if test $krb5_cv_inet6 = yes || test "$krb5_cv_inet6_with_dinet6" = yes; then
  if test "$krb5_cv_inet6_with_dinet6" = yes; then
    AC_DEFINE(INET6,1,[May need to be defined to enable IPv6 support, for example on IRIX])
  fi
  AC_DEFINE(KRB5_USE_INET6,1,[Define if we should compile in IPv6 support (even if we can't use it at run time)])
fi
])dnl
dnl
dnl Generic File existence tests
dnl 
dnl K5_AC_CHECK_FILE(FILE, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN(K5_AC_CHECK_FILE,
[AC_REQUIRE([AC_PROG_CC])dnl
dnl Do the transliteration at runtime so arg 1 can be a shell variable.
ac_safe=`echo "$1" | sed 'y%./+-%__p_%'`
AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_file_$ac_safe,
[if test "$cross_compiling" = yes; then
  errprint(__file__:__line__: warning: Cannot check for file existence when cross compiling
)dnl
  AC_MSG_ERROR(Cannot check for file existence when cross compiling)
else
  if test -r $1; then
    eval "ac_cv_file_$ac_safe=yes"
  else
    eval "ac_cv_file_$ac_safe=no"
  fi
fi])dnl
if eval "test \"`echo '$ac_cv_file_'$ac_safe`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(no)
ifelse([$3], , , [$3
np])dnl
fi
])dnl
dnl
dnl K5_AC_CHECK_FILES(FILE... [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
AC_DEFUN(K5_AC_CHECK_FILES,
[AC_REQUIRE([AC_PROG_CC])dnl
for ac_file in $1
do
K5_AC_CHECK_FILE($ac_file,
[changequote(, )dnl
  ac_tr_file=HAVE`echo $ac_file | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_file) $2], $3)dnl
done
])dnl
AC_DEFUN(KRB5_AC_CHECK_FOR_CFLAGS,[
AC_BEFORE([$0],[AC_PROG_CC])
krb5_ac_cflags_set=${CFLAGS+set}
krb5_ac_warn_cflags_set=${WARN_CFLAGS+set}
])
dnl
AC_DEFUN(TRY_WARN_CC_FLAG,[dnl
  cachevar=`echo "krb5_cv_cc_flag_$1" | sed s/[[^a-zA-Z0-9_]]/_/g`
  AC_CACHE_CHECK([if C compiler supports $1], [$cachevar],
  [# first try without, then with
  AC_TRY_COMPILE([], 1;,
    [old_cflags="$CFLAGS"
     CFLAGS="$CFLAGS $1"
     AC_TRY_COMPILE([], 1;, eval $cachevar=yes, eval $cachevar=no)
     CFLAGS="$old_cflags"],
    [AC_MSG_ERROR(compiling simple test program with $CFLAGS failed)])])
  if eval test '"${'$cachevar'}"' = yes; then
    WARN_CFLAGS="$WARN_CFLAGS $1"
  fi
  eval flag_supported='${'$cachevar'}'
])dnl
dnl
AC_DEFUN(WITH_CC,[dnl
AC_REQUIRE([KRB5_AC_CHECK_FOR_CFLAGS])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CXX])dnl
if test $ac_cv_c_compiler_gnu = yes ; then
     HAVE_GCC=yes
     else HAVE_GCC=
fi
AC_SUBST(HAVE_GCC)
AC_CACHE_CHECK([for GNU linker], krb5_cv_prog_gnu_ld,
[krb5_cv_prog_gnu_ld=no
if test "$GCC" = yes; then
  if AC_TRY_COMMAND([$CC -Wl,-v 2>&1 dnl
			| grep "GNU ld" > /dev/null]); then
    krb5_cv_prog_gnu_ld=yes
  fi
fi])
# -Wno-long-long, if needed, for k5-platform.h without inttypes.h etc.
extra_gcc_warn_opts="-Wall -Wcast-qual -Wcast-align -Wshadow"
# -Wmissing-prototypes
if test "$GCC" = yes ; then
  if test "x$krb5_ac_warn_cflags_set" = xset ; then
    AC_MSG_NOTICE(not adding extra gcc warning flags because WARN_CFLAGS was set)
  else
    AC_MSG_NOTICE(adding extra warning flags for gcc)
    WARN_CFLAGS="$WARN_CFLAGS $extra_gcc_warn_opts -Wmissing-prototypes"
    if test "`uname -s`" = Darwin ; then
      AC_MSG_NOTICE(skipping pedantic warnings on Darwin)
    elif test "`uname -s`" = Linux ; then
      AC_MSG_NOTICE(skipping pedantic warnings on Linux)
    else
      WARN_CFLAGS="$WARN_CFLAGS -pedantic"
    fi
    # Currently, G++ does not support -Wno-format-zero-length.
    TRY_WARN_CC_FLAG(-Wno-format-zero-length)
    # Other flags here may not be supported on some versions of
    # gcc that people want to use.
    for flag in overflow strict-overflow missing-format-attribute missing-prototypes return-type missing-braces parentheses switch unused-function unused-label unused-variable unused-value unknown-pragmas sign-compare newline-eof ; do
      TRY_WARN_CC_FLAG(-W$flag)
    done
    #  old-style-definition? generates many, many warnings
    #
    # Warnings that we'd like to turn into errors on versions of gcc
    # that support promoting only specific warnings to errors, but
    # we'll take as warnings on older compilers.  (If such a warning
    # is added after the -Werror=foo feature, you can just put
    # error=foo in the above list, and skip the test for the
    # warning-only form.)  At least in some versions, -Werror= doesn't
    # seem to make the conditions actual errors, but still issues
    # warnings; I guess we'll take what we can get.
    #
    # We're currently targeting C89+, not C99, so disallow some
    # constructs.
    for flag in declaration-after-statement variadic-macros ; do
      TRY_WARN_CC_FLAG(-Werror=$flag)
      if test "$flag_supported" = no; then
        TRY_WARN_CC_FLAG(-W$flag)
      fi
    done
    #  missing-prototypes? maybe someday
    #
  fi
  if test "`uname -s`" = Darwin ; then
    # Someday this should be a feature test.
    # One current (Jaguar = OS 10.2) problem:
    # Archive library with foo.o undef sym X and bar.o common sym X,
    # if foo.o is pulled in at link time, bar.o may not be, causing
    # the linker to complain.
    # Dynamic library problems too?
    case "$CC $CFLAGS" in
    *-fcommon*) ;; # why someone would do this, I don't know
    *-fno-common*) ;; # okay, they're already doing the right thing
    *)
      AC_MSG_NOTICE(disabling the use of common storage on Darwin)
      CFLAGS="$CFLAGS -fno-common"
      ;;
    esac
    case "$LD $LDFLAGS" in
    *-Wl,-search_paths_first*) ;;
    *) LDFLAGS="${LDFLAGS} -Wl,-search_paths_first" ;;
    esac
  fi
else
  if test "`uname -s`" = AIX ; then
    # Using AIX but not GCC, assume native compiler.
    # The native compiler appears not to give a nonzero exit
    # status for certain classes of errors, like missing arguments
    # in function calls.  Let's try to fix that with -qhalt=e.
    case "$CC $CFLAGS" in
      *-qhalt=*) ;;
      *)
	CFLAGS="$CFLAGS -qhalt=e"
	AC_MSG_NOTICE(adding -qhalt=e for better error reporting)
	;;
    esac
    # Also, the optimizer isn't turned on by default, which means
    # the static inline functions get left in random object files,
    # leading to references to pthread_mutex_lock from anything that
    # includes k5-int.h whether it uses threads or not.
    case "$CC $CFLAGS" in
      *-O*) ;;
      *)
	CFLAGS="$CFLAGS -O"
	AC_MSG_NOTICE(adding -O for inline thread-support function elimination)
	;;
    esac
  fi
  if test "`uname -s`" = SunOS ; then
    # Using Solaris but not GCC, assume Sunsoft compiler.
    # We have some error-out-on-warning options available.
    # Sunsoft 12 compiler defaults to -xc99=all, it appears, so "inline"
    # works, but it also means that declaration-in-code warnings won't
    # be issued.
    # -v -fd -errwarn=E_DECLARATION_IN_CODE ...
    WARN_CFLAGS="-errtags=yes -errwarn=E_BAD_PTR_INT_COMBINATION,E_BAD_PTR_INT_COMB_ARG,E_PTR_TO_VOID_IN_ARITHMETIC,E_NO_IMPLICIT_DECL_ALLOWED,E_ATTRIBUTE_PARAM_UNDEFINED"
  fi
fi
AC_SUBST(WARN_CFLAGS)
])dnl
dnl
dnl V5_AC_OUTPUT_MAKEFILE
dnl
AC_DEFUN(V5_AC_OUTPUT_MAKEFILE,
[ifelse($1, , [_V5_AC_OUTPUT_MAKEFILE(.,$2)],[_V5_AC_OUTPUT_MAKEFILE($1,$2)])])
dnl
define(_V5_AC_OUTPUT_MAKEFILE,
[ifelse($2, , ,AC_CONFIG_FILES($2))
AC_FOREACH([DIR], [$1],dnl
 [AC_CONFIG_FILES(DIR[/Makefile:$srcdir/pre.in:]DIR[/Makefile.in:]DIR[/deps:$srcdir/post.in])])
AC_OUTPUT])dnl
dnl
dnl
dnl KRB5_SOCKADDR_SA_LEN: define HAVE_SA_LEN if sockaddr contains the sa_len
dnl component
dnl
AC_DEFUN([KRB5_SOCKADDR_SA_LEN],[ dnl
AC_CHECK_MEMBER(struct sockaddr.sa_len,
  AC_DEFINE(HAVE_SA_LEN,1,[Define if struct sockaddr contains sa_len])
,,[#include <sys/types.h>
#include <sys/socket.h>])])
dnl
dnl
dnl CHECK_UTMP: check utmp structure and functions
dnl
AC_DEFUN(CHECK_UTMP,[
AC_CHECK_MEMBERS([struct utmp.ut_pid, struct utmp.ut_type, struct utmp.ut_host, struct utmp.ut_exit],,,
[#include <sys/types.h>
#include <utmp.h>])

# Define the names actually used in the krb5 code currently:
if test $ac_cv_member_struct_utmp_ut_pid = no; then
  AC_DEFINE(NO_UT_PID,1,[Define if ut_pid field not found])
fi
if test $ac_cv_member_struct_utmp_ut_type = no; then
  AC_DEFINE(NO_UT_TYPE,1,[Define if ut_type field not found])
fi
if test $ac_cv_member_struct_utmp_ut_host = no; then
  AC_DEFINE(NO_UT_HOST,1,[Define if ut_host field not found])
fi
if test $ac_cv_member_struct_utmp_ut_exit = no; then
  AC_DEFINE(NO_UT_EXIT,1,[Define if ut_exit field not found])
fi

AC_CHECK_FUNCS(setutent setutxent updwtmp updwtmpx)
])dnl
dnl
dnl
AC_DEFUN(KRB5_AC_NEED_DAEMON, [
KRB5_NEED_PROTO([#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif],daemon,1)])dnl
dnl
dnl Check if stdarg or varargs is available *and compiles*; prefer stdarg.
dnl (This was sent to djm for incorporation into autoconf 3/12/1996.  KR)
dnl
AC_DEFUN(AC_HEADER_STDARG, [

AC_MSG_CHECKING([for stdarg.h])
AC_CACHE_VAL(ac_cv_header_stdarg_h,
[AC_TRY_COMPILE([#include <stdarg.h>], [
  } /* ac_try_compile will have started a function body */
  int aoeu (char *format, ...) {
    va_list v;
    int i;
    va_start (v, format);
    i = va_arg (v, int);
    va_end (v);
],ac_cv_header_stdarg_h=yes,ac_cv_header_stdarg_h=no)])dnl
AC_MSG_RESULT($ac_cv_header_stdarg_h)
if test $ac_cv_header_stdarg_h = yes; then
  AC_DEFINE(HAVE_STDARG_H, 1, [Define if stdarg available and compiles])
else

AC_MSG_CHECKING([for varargs.h])
AC_CACHE_VAL(ac_cv_header_varargs_h,
[AC_TRY_COMPILE([#include <varargs.h>],[
  } /* ac_try_compile will have started a function body */
  int aoeu (va_alist) va_dcl {
    va_list v;
    int i;
    va_start (v);
    i = va_arg (v, int);
    va_end (v);
],ac_cv_header_varargs_h=yes,ac_cv_header_varargs_h=no)])dnl
AC_MSG_RESULT($ac_cv_header_varargs_h)
if test $ac_cv_header_varargs_h = yes; then
  AC_DEFINE(HAVE_VARARGS_H, 1, [Define if varargs available and compiles])
else
  AC_MSG_ERROR(Neither stdarg nor varargs compile?)
fi

fi dnl stdarg test failure

])dnl
dnl
dnl AC_KRB5_TCL_FIND_CONFIG (uses tcl_dir)
dnl
AC_DEFUN(AC_KRB5_TCL_FIND_CONFIG,[
AC_REQUIRE([KRB5_LIB_AUX])dnl
AC_MSG_CHECKING(for tclConfig.sh)
dnl On Debian, we might be given --with-tcl=/usr, or tclsh might
dnl point us to /usr/lib/tcl8.4; either way, we need to find
dnl /usr/lib/tcl8.4/tclConfig.sh.
dnl On NetBSD, we might be given --with-tcl=/usr/pkg, or tclsh
dnl might point us to /usr/pkg/lib/tcl8.4; we need to find
dnl /usr/pkg/lib/tclConfig.sh.
if test -r "$tcl_dir/lib/tclConfig.sh" ; then
  tcl_conf="$tcl_dir/lib/tclConfig.sh"
elif test -r "$tcl_dir/tclConfig.sh" ; then
  tcl_conf="$tcl_dir/tclConfig.sh"
elif test -r "$tcl_dir/../tclConfig.sh" ; then
  tcl_conf="$tcl_dir/../tclConfig.sh"
else
  tcl_conf=
  lib="$tcl_dir/lib"
  changequote(<<,>>)dnl
  for d in "$lib" "$lib"/tcl7.[0-9] "$lib"/tcl8.[0-9] ; do
    if test -r "$d/tclConfig.sh" ; then
      tcl_conf="$tcl_conf $d/tclConfig.sh"
    fi
  done
  changequote([,])dnl
fi
if test -n "$tcl_conf" ; then
  AC_MSG_RESULT($tcl_conf)
else
  AC_MSG_RESULT(not found)
fi
tcl_ok_conf=
tcl_vers_maj=
tcl_vers_min=
old_CPPFLAGS=$CPPFLAGS
old_LIBS=$LIBS
old_LDFLAGS=$LDFLAGS
if test -n "$tcl_conf" ; then
  for file in $tcl_conf ; do
    TCL_MAJOR_VERSION=x ; TCL_MINOR_VERSION=x
    AC_MSG_CHECKING(Tcl info in $file)
    . $file
    v=$TCL_MAJOR_VERSION.$TCL_MINOR_VERSION
    if test -z "$tcl_vers_maj" \
	|| test "$tcl_vers_maj" -lt "$TCL_MAJOR_VERSION" \
	|| test "$tcl_vers_maj" = "$TCL_MAJOR_VERSION" -a "$tcl_vers_min" -lt "$TCL_MINOR_VERSION" ; then
      for incdir in "$TCL_PREFIX/include/tcl$v" "$TCL_PREFIX/include" ; do
	if test -r "$incdir/tcl.h" -o -r "$incdir/tcl/tcl.h" ; then
	  CPPFLAGS="$old_CPPFLAGS -I$incdir"
	  break
	fi
      done
      LIBS="$old_LIBS `eval echo x $TCL_LIB_SPEC $TCL_LIBS | sed 's/^x//'`"
      LDFLAGS="$old_LDFLAGS $TCL_LD_FLAGS"
      AC_TRY_LINK( , [Tcl_CreateInterp ();],
	tcl_ok_conf=$file
	tcl_vers_maj=$TCL_MAJOR_VERSION
	tcl_vers_min=$TCL_MINOR_VERSION
	AC_MSG_RESULT($v - working),
	AC_MSG_RESULT($v - compilation failed)
      )
    else
      AC_MSG_RESULT(older version $v)
    fi
  done
fi
CPPFLAGS=$old_CPPFLAGS
LIBS=$old_LIBS
LDFLAGS=$old_LDFLAGS
tcl_header=no
tcl_lib=no
if test -n "$tcl_ok_conf" ; then
  . $tcl_ok_conf
  TCL_INCLUDES=
  for incdir in "$TCL_PREFIX/include/tcl$v" "$TCL_PREFIX/include" ; do
    if test -r "$incdir/tcl.h" -o -r "$incdir/tcl/tcl.h" ; then
      if test "$incdir" != "/usr/include" ; then
        TCL_INCLUDES=-I$incdir
      fi
      break
    fi
  done
  # Need eval because the first-level expansion could reference
  # variables like ${TCL_DBGX}.
  eval TCL_LIBS='"'$TCL_LIB_SPEC $TCL_LIBS $TCL_DL_LIBS'"'
  TCL_LIBPATH="-L$TCL_EXEC_PREFIX/lib"
  TCL_RPATH=":$TCL_EXEC_PREFIX/lib"
  if test "$DEPLIBEXT" != "$SHLIBEXT" && test -n "$RPATH_FLAG"; then
    TCL_MAYBE_RPATH='$(RPATH_FLAG)'"$TCL_EXEC_PREFIX/lib$RPATH_TAIL"
  else
    TCL_MAYBE_RPATH=
  fi
  CPPFLAGS="$old_CPPFLAGS $TCL_INCLUDES"
  AC_CHECK_HEADER(tcl.h,AC_DEFINE(HAVE_TCL_H,1,[Define if tcl.h is available]) tcl_header=yes)
  if test $tcl_header=no; then
     AC_CHECK_HEADER(tcl/tcl.h,AC_DEFINE(HAVE_TCL_TCL_H,1,[Define if tcl/tcl.h is available]) tcl_header=yes)
  fi
  CPPFLAGS="$old_CPPFLAGS"
  tcl_lib=yes
else
  # If we read a tclConfig.sh file, it probably set this.
  TCL_LIBS=
fi  
AC_SUBST(TCL_INCLUDES)
AC_SUBST(TCL_LIBS)
AC_SUBST(TCL_LIBPATH)
AC_SUBST(TCL_RPATH)
AC_SUBST(TCL_MAYBE_RPATH)
])dnl
dnl
dnl AC_KRB5_TCL_TRYOLD
dnl attempt to use old search algorithm for locating tcl
dnl
AC_DEFUN(AC_KRB5_TCL_TRYOLD, [
AC_REQUIRE([KRB5_AC_FIND_DLOPEN])
AC_MSG_WARN([trying old tcl search code])
if test "$with_tcl" != yes -a "$with_tcl" != no; then
	TCL_INCLUDES=-I$with_tcl/include
	TCL_LIBPATH=-L$with_tcl/lib
	TCL_RPATH=:$with_tcl/lib
fi
if test "$with_tcl" != no ; then
	krb5_save_CPPFLAGS="$CPPFLAGS"
	krb5_save_LDFLAGS="$LDFLAGS"
	CPPFLAGS="$CPPFLAGS $TCL_INCLUDES"
	LDFLAGS="$LDFLAGS $TCL_LIBPATH"
	tcl_header=no
	AC_CHECK_HEADER(tcl.h,AC_DEFINE(HAVE_TCL_H,1,[Define if tcl.h found]) tcl_header=yes)
	if test $tcl_header=no; then
	   AC_CHECK_HEADER(tcl/tcl.h,AC_DEFINE(HAVE_TCL_TCL_H,1,[Define if tcl/tcl.h found]) tcl_header=yes)
	fi

	if test $tcl_header = yes ; then
		tcl_lib=no

		if test $tcl_lib = no; then
			AC_CHECK_LIB(tcl8.0, Tcl_CreateCommand, 
				TCL_LIBS="$TCL_LIBS -ltcl8.0 -lm $DL_LIB $LIBS"
				tcl_lib=yes,,-lm $DL_LIB)
		fi
		if test $tcl_lib = no; then
			AC_CHECK_LIB(tcl7.6, Tcl_CreateCommand, 
				TCL_LIBS="$TCL_LIBS -ltcl7.6 -lm $DL_LIB $LIBS"
				tcl_lib=yes,,-lm $DL_LIB)
		fi
		if test $tcl_lib = no; then
			AC_CHECK_LIB(tcl7.5, Tcl_CreateCommand, 
				TCL_LIBS="$TCL_LIBS -ltcl7.5 -lm $DL_LIB $LIBS"
				tcl_lib=yes,,-lm $DL_LIB)

		fi
		if test $tcl_lib = no ; then
			AC_CHECK_LIB(tcl, Tcl_CreateCommand, 
				TCL_LIBS="$TCL_LIBS -ltcl -lm $DL_LIB $LIBS"
				tcl_lib=yes,,-lm $DL_LIB)

		fi
		if test $tcl_lib = no ; then		
			AC_MSG_WARN("tcl.h found but not library")
		fi
	else
		AC_MSG_WARN(Could not find Tcl which is needed for the kadm5 tests)
		TCL_LIBS=
	fi
	CPPFLAGS="$krb5_save_CPPFLAGS"
	LDFLAGS="$krb5_save_LDFLAGS"
	AC_SUBST(TCL_INCLUDES)
	AC_SUBST(TCL_LIBS)
	AC_SUBST(TCL_LIBPATH)
	AC_SUBST(TCL_RPATH)
else
	AC_MSG_RESULT("Not looking for Tcl library")
fi
])dnl
dnl
dnl AC_KRB5_TCL - determine if the TCL library is present on system
dnl
AC_DEFUN(AC_KRB5_TCL,[
TCL_INCLUDES=
TCL_LIBPATH=
TCL_RPATH=
TCL_LIBS=
TCL_WITH=
tcl_dir=
AC_ARG_WITH(tcl,
[  --with-tcl=path         where Tcl resides], , with_tcl=try)
if test "$with_tcl" = no ; then
  true
elif test "$with_tcl" = yes -o "$with_tcl" = try ; then
  tcl_dir=/usr
  if test ! -r /usr/lib/tclConfig.sh; then
    cat >> conftest <<\EOF
puts "tcl_dir=$tcl_library"
EOF
    if tclsh conftest >conftest.out 2>/dev/null; then
      if grep tcl_dir= conftest.out >/dev/null 2>&1; then
        t=`sed s/tcl_dir=// conftest.out`
        tcl_dir=$t
      fi
    fi # tclsh ran script okay
  rm -f conftest conftest.out
  fi # no /usr/lib/tclConfig.sh
else
  tcl_dir=$with_tcl
fi
if test "$with_tcl" != no ; then
  AC_KRB5_TCL_FIND_CONFIG
  if test $tcl_lib = no ; then
    if test "$with_tcl" != try ; then
      AC_KRB5_TCL_TRYOLD
    else
      AC_MSG_WARN(Could not find Tcl which is needed for some tests)
    fi
  fi
fi
# If "yes" or pathname, error out if not found.
if test "$with_tcl" != no -a "$with_tcl" != try ; then
  if test "$tcl_header $tcl_lib" != "yes yes" ; then
    AC_MSG_ERROR(Could not find Tcl)
  fi
fi
])dnl
dnl
dnl Check if we need the prototype for a function - we give it a bogus 
dnl prototype and if it complains - then a valid prototype exists on the 
dnl system.
dnl
dnl KRB5_NEED_PROTO(includes, function, [bypass])
dnl if $3 set, don't see if library defined. 
dnl Useful for case where we will define in libkrb5 the function if need be
dnl but want to know if a prototype exists in either case on system.
dnl
AC_DEFUN([KRB5_NEED_PROTO], [
ifelse([$3], ,[if test "x$ac_cv_func_$2" = xyes; then])
AC_CACHE_CHECK([if $2 needs a prototype provided], krb5_cv_func_$2_noproto,
AC_TRY_COMPILE([$1],
[#undef $2
struct k5foo {int foo; } xx;
extern int $2 (struct k5foo*);
$2(&xx);
],
krb5_cv_func_$2_noproto=yes,krb5_cv_func_$2_noproto=no))
if test $krb5_cv_func_$2_noproto = yes; then
	AC_DEFINE([NEED_]translit($2, [a-z], [A-Z])[_PROTO], 1, dnl
[define if the system header files are missing prototype for $2()])
fi
ifelse([$3], ,[fi])
])dnl
dnl
dnl =============================================================
dnl Internal function for testing for getsockname arguments
dnl
AC_DEFUN([TRY_GETSOCK_INT],[
krb5_lib_var=`echo "$1 $2" | sed 'y% ./+-*%___p_p%'`
AC_MSG_CHECKING([if getsockname() takes arguments $1 and $2])
AC_CACHE_VAL(krb5_cv_getsockname_proto_$krb5_lib_var,
[
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>
extern int getsockname(int, $1, $2);
],,eval "krb5_cv_getsockname_proto_$krb5_lib_var=yes",
    eval "krb5_cv_getsockname_proto_$krb5_lib_var=no")])
if eval "test \"`echo '$krb5_cv_getsockname_proto_'$krb5_lib_var`\" = yes"; then
	AC_MSG_RESULT(yes)
	sock_set=yes; res1="$1"; res2="$2"
else
	AC_MSG_RESULT(no)
fi
])dnl
dnl
dnl Determines the types of the second and third arguments to getsockname().
dnl
AC_DEFUN([KRB5_GETSOCKNAME_ARGS],[
sock_set=no
for sock_arg1 in "struct sockaddr *" "void *"
do
  for sock_arg2 in "size_t *" "int *" "socklen_t *"
  do
	if test $sock_set = no; then
	  TRY_GETSOCK_INT($sock_arg1, $sock_arg2)
	fi
  done 
done
if test "$sock_set" = no; then
  AC_MSG_NOTICE(assuming struct sockaddr and socklen_t for getsockname args)
  res1="struct sockaddr *"
  res2="socklen_t *"
fi
res1=`echo "$res1" | tr -d '*' | sed -e 's/ *$//'`
res2=`echo "$res2" | tr -d '*' | sed -e 's/ *$//'`
AC_DEFINE_UNQUOTED([GETSOCKNAME_ARG2_TYPE],$res1,[Type of pointer target for argument 2 to getsockname])
AC_DEFINE_UNQUOTED([GETSOCKNAME_ARG3_TYPE],$res2,[Type of pointer target for argument 3 to getsockname])
])dnl
dnl
dnl KRB5_AC_PRIOCNTL_HACK
dnl
dnl
AC_DEFUN([KRB5_AC_PRIOCNTL_HACK],
[AC_REQUIRE([AC_PROG_AWK])dnl
AC_REQUIRE([AC_LANG_COMPILER_REQUIRE])dnl
AC_CACHE_CHECK([whether to use priocntl hack], [krb5_cv_priocntl_hack],
[case $krb5_cv_host in
*-*-solaris2.9*)
	if test "$cross_compiling" = yes; then
		krb5_cv_priocntl_hack=yes
	else
		# Solaris patch 117171-11 (sparc) or 117172-11 (x86)
		# fixes the Solaris 9 bug where final pty output
		# gets lost on close.
		if showrev -p | $AWK 'BEGIN { e = 1 }
/Patch: 11717[[12]]/ { x = index[]([$]2, "-");
if (substr[]([$]2, x + 1, length([$]2) - x) >= 11)
{ e = 0 } else { e = 1 } }
END { exit e; }'; then
			krb5_cv_priocntl_hack=no
		else
			krb5_cv_priocntl_hack=yes
		fi
	fi
	;;
*)
	krb5_cv_priocntl_hack=no
	;;
esac])
if test "$krb5_cv_priocntl_hack" = yes; then
	PRIOCNTL_HACK=1
else
	PRIOCNTL_HACK=0
fi
AC_SUBST(PRIOCNTL_HACK)])dnl
dnl
dnl KRB5_AC_LIBUTIL
dnl
dnl Check for libutil, for NetBSD, et al.; needed for openpty() and
dnl logwtmp() on some platforms.
dnl
AC_DEFUN([KRB5_AC_LIBUTIL],
	[AC_CHECK_LIB(util, main,
		[AC_DEFINE(HAVE_LIBUTIL,1,[Define if util library is available with openpty, logwtmp, etc])
  UTIL_LIB=-lutil])dnl
AC_SUBST(UTIL_LIB)
])dnl
