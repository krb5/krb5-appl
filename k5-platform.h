/*
 * k5-platform.h
 *
 * Copyright 2003, 2004, 2005, 2007, 2008, 2009 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.	Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * Some platform-dependent definitions to sync up the C support level.
 * Some to a C99-ish level, some related utility code.
 *
 * Currently:
 * + strlcpy, strlcat
 * + [v]asprintf
 * + detecting snprintf overflows
 */

#ifndef K5_PLATFORM_H
#define K5_PLATFORM_H

#include "autoconf.h"
#include <string.h>
#include <stdarg.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

/* Provide strlcpy/strlcat interfaces. */
#ifndef HAVE_STRLCPY
#define strlcpy krb5int_strlcpy
#define strlcat krb5int_strlcat
extern size_t krb5int_strlcpy(char *dst, const char *src, size_t siz);
extern size_t krb5int_strlcat(char *dst, const char *src, size_t siz);
#endif

#ifndef HAVE_VASPRINTF

extern int krb5int_vasprintf(char **, const char *, va_list)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 2, 0)))
#endif
    ;
extern int krb5int_asprintf(char **, const char *, ...)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 2, 3)))
#endif
    ;

#define vasprintf krb5int_vasprintf
/* Assume HAVE_ASPRINTF iff HAVE_VASPRINTF.  */
#define asprintf krb5int_asprintf

#elif defined(NEED_VASPRINTF_PROTO)

extern int vasprintf(char **, const char *, va_list)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 2, 0)))
#endif
    ;
extern int asprintf(char **, const char *, ...)
#if !defined(__cplusplus) && (__GNUC__ > 2)
    __attribute__((__format__(__printf__, 2, 3)))
#endif
    ;

#endif /* have vasprintf and prototype? */

/* Return true if the snprintf return value RESULT reflects a buffer
   overflow for the buffer size SIZE.

   We cast the result to unsigned int for two reasons.  First, old
   implementations of snprintf (such as the one in Solaris 9 and
   prior) return -1 on a buffer overflow.  Casting the result to -1
   will convert that value to UINT_MAX, which should compare larger
   than any reasonable buffer size.  Second, comparing signed and
   unsigned integers will generate warnings with some compilers, and
   can have unpredictable results, particularly when the relative
   widths of the types is not known (size_t may be the same width as
   int or larger).
*/
#define SNPRINTF_OVERFLOW(result, size) \
    ((unsigned int)(result) >= (size_t)(size))

#endif /* K5_PLATFORM_H */
