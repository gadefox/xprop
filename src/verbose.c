/*
 * Copyright 1990, 1998  The Open Group
 * Copyright (c) 2000  The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of The Open Group shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization
 * from The Open Group.
 */

#include "verbose.h"
#include "strutil.h"
#include "misc.h"


const char *prog_name;

/* messages */
const char msg_arg_unknown_char[]  = "unrecognized argument: -%c";
const char msg_arg_unknown_param[] = "%s: unrecognized parameter '%s'";
const char msg_arg_unknown[]       = "unrecognized argument: %s";
const char msg_arg_missing[]       = "%s requires an argument";
const char msg_arg_only_windows[]  = "--%s works only on windows, not fonts";

const char msg_invalid_number[]    = "%s: invalid #: %s";
const char msg_missing_char[]      = "%s: missing '%c'";
const char msg_missing[]           = "%s: missing %s";
const char msg_invalid_size[]      = "%s: invalid size: %d";
const char msg_invalid_char[]      = "%s: invalid char: %c";
const char msg_empty[]             = "%s: cannot be empty";
const char msg_too_long[]          = "%s: too long";

const char msg_missing_format[]    = "%s: missing format";
const char msg_missing_dformat[]   = "%s: missing dformat";

const char msg_invalid_winid[]     = "window: invalid id # %s";
const char msg_out_of_memory[]     = "out of memory";

const char msg_prop_not_found[]    = "%s: property not found";
const char msg_prop_char_size[]    = "can't use format character '%c' with any size except %d";


void
verbose_color_begin (FILE *file, VerboseColor color)
{
    fprintf (file, "\033[%d;1m", color);
}

void
verbose_color_end (FILE *file)
{
    fputs ("\033[0m", file);
}

void verbose_color (FILE *file, const char *str, VerboseColor color)
{
    verbose_color_begin (file, color);
    fputs (str, file);
    verbose_color_end (file);
}

void
verbose_prefix (FILE *file, const char *prefix)
{
    /* "$prefix: " */
    fputs (prefix, file);
    fputc (':', file);
    fputc (' ', file);
}

void
verbose_color_prefix (FILE *file, const char *prefix, VerboseColor color)
{
    verbose_color_begin (file, color);
    verbose_prefix (file, prefix);
    verbose_color_end (file);
}

void
verbose (FILE *file, const char *prefix, VerboseColor color, const char *format, va_list params)
{
    verbose_prefix (file, prog_name);
    verbose_color_prefix (file, prefix, color);
    vfprintf (file, format, params);
    fputc ('\n', file);
}

void
verbose_string (const char *name, const char *val)
{
    if ( name != NULL )
        verbose_color_prefix (stdout, name, VerboseCyan);

    verbose_s (val);
    verbose_newline ();
}

void
verbose_v (const char **v, int size)
{
    const char *s;
    
    if ( size == -1 ) {
        for ( s = *v; s != NULL; s = *++v ) {
            verbose_s (s);
            verbose_newline ();
        }
    } else {
         while ( size-- != 0 ) {
            verbose_s (*v++);
            verbose_newline ();
        }
    }
}

void
verbose_spaces (unsigned int count)
{
    while (count != 0 ) {
        verbose_c (' ');
        count--;
    }
}

void
help (const char *format, ...)
{
   va_list params;

    va_start (params, format);
    verbose (stdout, "HELP", VerboseMagenda, format, params);
    va_end (params);
}

void
warn (const char *format, ...)
{
    va_list params;

    va_start (params, format);
    verbose (stderr, "WARNING", VerboseYellow, format, params);
    va_end (params);
}

void
error (const char *format, ...)
{
    va_list params;

    va_start (params, format);
    verbose (stderr, "ERROR", VerboseRed, format, params);
    va_end (params);
}

void
info (const char *format, ...)
{
    va_list params;

    va_start (params, format);
    verbose (stdout, "INFO", VerboseGreen, format, params);
    va_end (params);
}

void
verbose_d (int value)
{
    char szval [MAX_NUM_SIZE];

    s_int (szval, value);
    verbose_s (szval);
}

void
verbose_color_int (int value, VerboseColor color)
{
    verbose_color_begin (stdout, color);
    verbose_d (value);
    verbose_color_end (stdout);
}

void
verbose_u (unsigned int value)
{
    char szval [MAX_NUM_SIZE];

    s_uint (szval, value);
    verbose_s (szval);
}

void
verbose_color_uint (unsigned int value, VerboseColor color)
{
    verbose_color_begin (stdout, color);
    verbose_u (value);
    verbose_color_end (stdout);
}
