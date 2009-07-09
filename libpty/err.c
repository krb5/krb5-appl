/*
 * err.c: libpty error strings
 * 
 * Copyright 2009 by the Massachusetts Institute of Technology.
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

#include "libpty.h"

/* Keep in sync with error codes in libpty.h. */
const char *error_table[] = {
    "Success",
    "Failed to unlock or grant streams pty.",
    "fstat of master pty failed",
    "All terminal ports in use",
    "Buffer to hold slave pty name is too short",
    "Failed to open slave side of pty",
    "Failed to chmod slave side of pty",
    "Unable to set controlling terminal",
    "Failed to chown slave side of pty",
    "Call to line_push failed to push streams on slave pty",
    "Failed to push stream on slave side of pty",
    "Failed to revoke slave side of pty",
    "Bad process type passed to pty_update_utmp",
    "Slave pty name is zero-length"
};

const char *pty_error_message(long code)
{
    if (code < 0 || code >= (sizeof(error_table) / sizeof(*error_table)))
        return "Unknown libpty error code";
    return error_table[code];
}
