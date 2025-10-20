/*
 * Copyright 1993, 1998  The Open Group
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

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "format.h"
#include "verbose.h"
#include "strutil.h"
#include "misc.h"
#include "print.h"
#include "args.h"


#define MAX_DELIM_SIZE  8


const char def_format[]  = "x";
const char def_dformat[] = "$@ ($#) = $0+\n";
const char def_delim[]   = ", ";

Format fmt_default;


int
Format_Print_Default (void)
{
    /* format, dformat format */
    Print ("default: ", ColorUndefined);
    verbose_newline ();

    if ( (a_flags & FlagRaw) == 0 )
        verbose_s ("  format  = ");

    if ( !Print_String (fmt_default.a.format, 0) )
        return False;

    verbose_newline ();

    if ( (a_flags & FlagRaw) == 0 )
        verbose_s ("  dformat = ");

    if ( !Print_String (fmt_default.d.format, 0) )
        return False;

    verbose_newline ();

    if ( (a_flags & FlagRaw) == 0 )
        verbose_s ("  delim   = ");

    if ( !Print_String (fmt_default.d.delim, 0) )
        return False;

    verbose_newline ();
    verbose_newline ();

    return True;
}

/*
 * Format atom
 */

int
Format_Atom_Is (const char *format)
{
    return isdigit (*format);
}

int
Format_Atom_Parse (FormatAtom *fa, const char *format)
{
    int u;
    char c;
    static char sz[] = "abciomstux";

    /* size */
    format = s_parse_uint (format, &u);
    if ( u == -1 ) {
        error (msg_invalid_number, format_arg, format);
        return False;
    }

    /* check for 0 | 8 | 16 | 32 */
    if ( u != 0 && u != 8 && u != 16 && u != 32 ) {
        error (msg_invalid_size, format_arg, u);
        return False;
    }

    fa->size = u;

    /* imax */
    u = strlen (format);
    if ( u == 0 ) {
        error (msg_empty, format_arg);
        return False;
    }
    
    fa->imax = u - 1;
    fa->format = format;

    /* check for a | b | c | i | o | m | s | t | u | x */
    while ( u-- != 0 ) {
        c = *format++;
        if ( strchr (sz, c) == NULL ) {
            error (msg_invalid_char, format_arg, c);
            return False;
        }
    }
    
    return True;
}


void
Format_Atom_Set_Default (FormatAtom *fa)
{
    /* "0x" */
    fa->size = 0;
    fa->imax = 0;  /* strlen - 1 */
    fa->format = def_format;
}

void
Format_Atom_Copy (FormatAtom *dst, FormatAtom *src)
{
    dst->format = src->format;
    dst->size = src->size;
    dst->imax = src->imax;
} 

int
Format_Atom_Dup (FormatAtom *dst, FormatAtom *src)
{
    dst->size = src->size;
    dst->imax = src->imax;
    dst->format = s_dup (src->format);

    return dst->format != NULL;
}

void
Format_Atom_Clear (FormatAtom *fa)
{
    fa->size = 0;
    fa->imax = 0;
    fa->format = NULL;
}

void
Format_Atom_Free (FormatAtom *fa)
{
    free ((char *) fa->format);
    fa->format = NULL;
}

char
Format_Atom_Get_Char (FormatAtom *fa, unsigned int i)
{
    /* Last character repeats forever... */
    if ( i > fa->imax )
        i = fa->imax;

    return fa->format [i];
}


/*
 * Format display
 */

void
Format_Display_Set_Default (FormatDisplay *fd)
{
    fd->format = def_dformat;
    fd->delim = def_delim;
}

const char *
Format_Display_Skip_Delim (const char *format)
{
    char c;

    for ( ;; ) {
        c = *++format;
        if ( c == '\0' )
            return format;

        if ( c == '^' )
            return format + 1;
    }
}

int
Format_Display_Copy_Delim (const char *format, char *delim, unsigned int size)
{
    char c;
    
    while ( size-- != 0 ) {
        c = *++format;
        if ( c == '\0' ) {
            error (msg_missing_char, format_arg, '^');
            return False;
        }

        if ( c == '^' ) {
            *delim = '\0';
            return True;
        }

        *delim++ = c;
    }

    error (msg_too_long, "delimiter");
    return False;
}

int
Format_Display_Parse_Delim (const char *format, char *delim, unsigned int size)
{
    char c;
    int i;

    while ( (c = *format++) != '\0' ) {
        if ( c != '$' )
            continue;

        /* $<c> */
        c = *format;
        if ( c == '\0' )
            break;

        /* $$ $@ $#, or $t */
        if ( c == '$' || c == '@' || c == '#' || c == 't' ) {
            format++;
            continue;
        }

        /* $^ */
        if ( c == '^' ) {
            if ( !Format_Display_Copy_Delim (format, delim, size) )
                return -1;

            return 1;
        }

        /* $n */
        format = s_parse_uint (format, &i);
        if ( i == -1 ) {
            error (msg_invalid_number, format_arg, format);
            return -1;
        }

        if ( *format != '+' )
            continue;

        /* $n+ */
        format++;
    }

    return 0;
}

int
Format_Display_Parse (FormatDisplay *fd, const char *format)
{
    char szdelim [MAX_DELIM_SIZE];
    int ret;

    /* delimiter parsing will detect all issues */
    ret = Format_Display_Parse_Delim (format, szdelim, countof (szdelim) );
    if ( ret == -1 )  /* error */
        return False;

    /* format */
    fd->format = format;

    /* delim */
    if ( ret == 0 ) {
        fd->delim = NULL;
        return True;
    }

    /* ret == 1: success */
    fd->delim = s_dup (szdelim);
    return fd->delim != NULL;
}

void
Format_Display_Copy (FormatDisplay *dst, FormatDisplay *src)
{
    dst->format = src->format;
    dst->delim = src->delim;
}

int
Format_Display_Dup (FormatDisplay *dst, FormatDisplay *src)
{
    if ( src->format == NULL ) {
        Format_Display_Clear (dst);
        return True;
    }

    /* format */
    dst->format = s_dup (src->format);
    if ( dst->format == NULL )
        return False;

    /* delim */
    if ( src->delim == NULL )
        return True;

    dst->delim = s_dup (src->delim);
    if ( dst->delim == NULL ) {
        Format_Display_Free (dst);
        return False;
    }

    return True;
}

void
Format_Display_Clear (FormatDisplay *fd)
{
    fd->format = NULL;
    fd->delim = NULL;
}

void
Format_Display_Free (FormatDisplay *fd)
{
    free ((char *) fd->format);
    fd->format = NULL;

    free ((char *) fd->delim);
    fd->delim = NULL;
}


/*
 * Format
 */

void
Format_Apply_Default (Format *fmt)
{
    /* format */
    if ( fmt->a.format == NULL )
        Format_Atom_Copy (&fmt->a, &fmt_default.a);

    /* dformat */
    if ( fmt->d.format == NULL )
        fmt->d.format = fmt_default.d.format;

    /* delim */
    if ( fmt->d.delim == NULL )
        fmt->d.delim = fmt_default.d.delim;
}

void
Format_Set_Default (Format *fmt)
{
    Format_Atom_Set_Default (&fmt->a);
    Format_Display_Set_Default (&fmt->d);
}

void
Format_Copy (Format *dst, Format *src)
{
    Format_Atom_Copy (&dst->a, &src->a);
    Format_Display_Copy (&dst->d, &src->d);
}

int
Format_Dup (Format *dst, Format *src)
{
    if ( !Format_Atom_Dup (&dst->a, &src->a) )
        return False;

    if ( !Format_Display_Dup (&dst->d, &src->d) ) {
        Format_Atom_Free (&dst->a);
        return False;
    }
    return True;
}

void
Format_Clear (Format *fmt)
{
    Format_Atom_Clear (&fmt->a);
    Format_Display_Clear (&fmt->d);
}

void
Format_Free (Format *fmt)
{
    Format_Atom_Free (&fmt->a);
    Format_Display_Free (&fmt->d);
}
