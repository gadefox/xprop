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

#ifndef _VERBOSE_H_
#define _VERBOSE_H_

#include <stdarg.h>
#include <stdio.h>


#define verbose_s(s)       (fputs ((s), stdout))
#define verbose_c(c)       (fputc ((c), stdout))
#define verbose_newline()  (verbose_c ('\n'))
#define verbose_comma()    { verbose_c (','); verbose_c (' '); }

#define error_s(s)         (fputs ((s), stderr))
#define error_c(c)         (fputc ((c), stderr))
#define error_newline()    (verbose_c ('\n'))
#define error_comma()      { verbose_c (','); verbose_c (' '); }


/* messages */
extern const char msg_arg_unknown_char[];
extern const char msg_arg_unknown_param[];
extern const char msg_arg_unknown[];
extern const char msg_arg_missing[];
extern const char msg_arg_only_windows[];

extern const char msg_invalid_number[];
extern const char msg_missing_char[];
extern const char msg_missing[];
extern const char msg_invalid_size[];
extern const char msg_invalid_char[];
extern const char msg_empty[];
extern const char msg_too_long[];

extern const char msg_invalid_winid[];
extern const char msg_out_of_memory[];

extern const char msg_prop_not_found[];
extern const char msg_prop_char_size[];


typedef enum {
    VerboseUndefined,
    VerboseBlack   = 30,
    VerboseRed     = 31,
    VerboseGreen   = 32,
    VerboseYellow  = 33,
    VerboseBlue    = 34,
    VerboseMagenda = 35,
    VerboseCyan    = 36,
    VerboseWhite   = 37
} VerboseColor;


extern const char *prog_name;


void verbose_color_begin (FILE *file, VerboseColor color);
void verbose_color_end (FILE *file);
void verbose_color (FILE *file, const char *str, VerboseColor color);
void verbose_prefix (FILE *file, const char *prefix);
void verbose_color_prefix (FILE *file, const char *prefix, VerboseColor color);
void verbose (FILE *file, const char *prefix, VerboseColor color, const char *err, va_list params);

void verbose_v (const char **v, int size);
void verbose_d (int value);
void verbose_u (unsigned int value);

void verbose_color_int (int value, VerboseColor color);
void verbose_color_uint (unsigned int value, VerboseColor color);
void verbose_spaces (unsigned int count);

void verbose_string (const char *name, const char *val);

void help (const char *format, ...);
void info (const char *format, ...);
void warn (const char *format, ...);
void error (const char *format, ...);


#endif /* _VERBOSE_H_ */
