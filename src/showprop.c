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

#include <stdlib.h>
#include <ctype.h>

#include "showprop.h"
#include "format.h"
#include "verbose.h"
#include "print.h"
#include "misc.h"
#include "strutil.h"


static const char field_not_available[] = "field not available";


const char *
Skip_Past_Right_Paren (const char *string)
{
    char c;
    int nesting = 0;

    for ( ;; )  {
        c = *string++;
        switch ( c ) {
            case '\0':
                error (msg_missing_char, ')');
                return string - 1;

            case ')':
                if ( nesting-- == 0 )
                    return string;
                break;

            case '(':
                nesting++;
                break;

            case '\\':
               string++;
               break;
        }
    }
}

int
Thunk_Show_Prop_Print (ThunkShowProp *t, char type, const char *delim)
{
    switch (type) {
        case 's':
            return Print_Len_String (t->extra, t->value, False);

        case 'u':
            return Print_Len_Unicode (t->extra, t->value);
            
        case 't':
            return Print_Len_Text (t->extra, t->value, t->encoding);
            
        case 'x':
            Print_Hex(t->value);
            break;

        case 'c':
            Print_Unsigned (t->value, ColorValue);
            break;

        case 'i':
            Print_Signed (t->value, ColorValue);
            break;

        case 'b':
            Print_Bool (t->value);
            break;

        case 'm':
            Print_Mask_Word (t->value, delim);
            break;

        case 'a':
            Print_Atom (t->value);
            break;

        case 'o':
            return Print_Icons ((const unsigned long *)t->extra, t->value);
    }

    return True;
}

int
Thunk_Show_Prop_Print_I (Thunks *thunks, Format *fmt, unsigned int i)
{
    char type;
    ThunkShowProp *t;

    if (i < thunks->nelements) {
        type = Format_Atom_Get_Char (&fmt->a, i);
        t = (ThunkShowProp *) Thunks_Get_Item (thunks, i);
        return Thunk_Show_Prop_Print (t, type, fmt->d.delim);
    }

    Print (field_not_available, ColorUndefined);
    return True;
}

int
Thunk_Show_Prop_Mask_Word (Thunks *thunks, FormatAtom *fa)
{
    unsigned int i;
    ThunkShowProp *t;
    const char *s;

    for ( i = fa->imax, s = fa->format, t = (ThunkShowProp *)thunks->items;
          i >= 0; i--, s++, t++ ) {  /* the condition should be i > 0 but $imax = strlen (format) - 1 */
        if ( *s == 'm' )
            return t->value;
    }
    return 0;
}

const char *
Handle_Backslash (const char *dformat)
{
    char c;
    int i;

    c = *dformat;
    switch (c) {
        case '\0':
            break;

        case 'n':
            dformat++;
            verbose_c ('\n');
            break;

        case 't':
            dformat++;
            verbose_c ('\t');
            break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
            dformat = Scan_Octal (dformat, &i);
            if ( dformat == NULL )
                return NULL;

            verbose_c ((int) i);
            break;

        default:
            dformat++;
            verbose_c (c);
            break;
    }

    return dformat;
}

const char *
Handle_Dollar (const char *dformat, NameType *nt, Thunks *thunks, Format *fmt)
{
    long i, seen;
    int c;
    VerboseColor color;
    ThunkShowProp *t;

    c = *dformat;
    switch ( c )
    {
        /* "$$" ~ '$' char */
        case '$':
            dformat++;
            verbose_c ('$');
            break;

            /* $@ ~ name */
        case '@':
            dformat++;

            if ( nt->flags & NameUndefined )
                color = ColorUndefined;
            else if ( nt->flags & PropertyWindow )
                color = ColorProperty;
            else
                color = ColorFont;
            Print (nt->name, color);
            break;

            /* $# ~ type */
        case '#':
            dformat++;

            if ( nt->type == NULL )
                break;

            color = nt->flags & TypeUndefined ? ColorUndefined : VerboseUndefined;
            Print (nt->type, color);
            break;

            /* $^<sep>^ */
        case '^':
            dformat = Format_Display_Skip_Delim (dformat);
            break;

            /* $t ~ TAB_SPACES */
        case 't':
            dformat++;
            verbose_spaces (TAB_SPACES);
            break;

        default:
            /* the formst is validated already */
            i = atoi (dformat);
            dformat = s_skip_digits (dformat);
            if ( *dformat != '+' ) {
                /* $n */
                if ( !Thunk_Show_Prop_Print_I (thunks, fmt, (int) i) )
                    return NULL;
                break;
            }

            /* $n+ */
            dformat++;

            for ( seen = False, t = (ThunkShowProp *) Thunks_Get_Item (thunks, i);
                i < thunks->nelements; i++, t++ ) {
                if (seen)
                    verbose_s (fmt->d.delim);
                else
                    seen = True;

                c = Format_Atom_Get_Char (&fmt->a, i);
                if ( !Thunk_Show_Prop_Print (t, c, fmt->d.delim) )
                    return NULL;
            }
            break;
    }

    return dformat;
}

int
Thunk_Show_Prop_Mask_Bit_I (Thunks *thunks, FormatAtom *fa, unsigned int i)
{
    long value;

    value = Thunk_Show_Prop_Mask_Word (thunks, fa);
    value = value & (1L << i);
    if ( value )
        return 1;

    return 0;
}

const char *
Thunk_Show_Prop_Scan_Term (const char *s, Thunks *thunks, FormatAtom *fa, int *value)
{
    int index;
    char c;
    ThunkShowProp *t;

    /* optional: we don't show the 'invalid #' message */
    s = s_parse_uint (s, value);
    if ( *value != -1 )
        return s;

    *value = 0;
    c = *s++;
    s = s_parse_uint (s, &index);
    if ( index == -1 ) {
        error (msg_invalid_number, "term", s);
        return NULL;
    }

    if ( c == '$' ) {
        /* $index cannot be less then 0 (see s_parse_uint fn implementation) */
        if ( index >= thunks->nelements ) {
            warn ( "$%d not available; using the last field", index);
            index = thunks->nelements - 1;
        }

        t = (ThunkShowProp *) Thunks_Get_Item (thunks, index);
        *value = t->value;
        return s;
    }

    if ( c == 'm' ) {
        /* $index is used as a temp variable */
        index = Thunk_Show_Prop_Mask_Bit_I (thunks, fa, index);
        *value = index;
        return s;
    }

    error ("invalid term: %s", s);
    return NULL;
}

const char *
Thunk_Show_Prop_Scan_Exp (const char *string, Thunks *thunks, FormatAtom *fa, int *value)
{
    int temp;
    char c;

    c = *string;
    if ( c == '(' ) {
        string = Thunk_Show_Prop_Scan_Exp (string + 1, thunks, fa, value);
        if ( string == NULL )
            return NULL;

        if ( *string != ')' ) {
            error (msg_missing_char, ')');
            return NULL;
        }

        return string + 1;
    }

    if ( c == '!' ) {
        string = Thunk_Show_Prop_Scan_Exp (string + 1, thunks, fa, value);
        /* string can be null when failed */

        *value = !*value;
    } else {
        string = Thunk_Show_Prop_Scan_Term (string, thunks, fa, value);
        if ( string == NULL )
            return NULL;

        if ( *string == '=' ) {
            string = Thunk_Show_Prop_Scan_Exp (string + 1, thunks, fa, &temp);
            /* string can be null when failed */

            *value = *value == temp;
        }
    }

    return string;
}

const char *
Handle_Question_Mark (const char *dformat, Thunks *thunks, FormatAtom *fa, int *question)
{
    int value;

    dformat = Thunk_Show_Prop_Scan_Exp (dformat, thunks, fa, &value);
    if ( dformat == NULL )
        return NULL;

    if ( *dformat++ != '(' ) {
        error (msg_missing_char, dformat);
        return NULL;
    }

    *question = value;
    return dformat;
}

int
Thunk_Show_Prop_Display_Property (NameType *nt, Thunks *thunks, Format *fmt)
{
    char c;
    const char *s;
    int question;
    int nesting = 0;
    
    s = fmt->d.format;
    while ( (c = *s++) != '\0' ) {
        switch ( c ) {
            case '\\':
                s = Handle_Backslash (s);
                if ( s == NULL )
                    return False;
                break;

            case '$':
                s = Handle_Dollar (s, nt, thunks, fmt);
                if ( s == NULL )
                    return False;
                break;

            case '?':
                s = Handle_Question_Mark (s, thunks, &fmt->a, &question);
                if ( s == NULL )
                    return False;

                if ( question ) {
                    /* ? result: true */
                    nesting++;
                    break;
                }
                /* ? result: false */
                s = Skip_Past_Right_Paren (s);
                break;

            case ')':
                if ( nesting == 0 ) {
                    verbose_c (')');
                    break;
                }

                if ( --nesting == 0 )
                    verbose_color_end (stdout);
                break;

            default:
                verbose_c (c);
                break;
        }
    }

    if ( nesting != 0 )
        error (msg_missing_char, ')');

    return True;
}

int
Extract_Value (const char **pointer, int *length, int size, int signedp)
{
    long value;

    switch (size) {
        case 8:
            if (signedp)
                value = * (const signed char *) *pointer;
            else
                value = * (const unsigned char *) *pointer;
            *pointer += 1;
            *length -= 1;
            break;

        case 16:
            if (signedp)
                value = * (const short *) *pointer;
            else
                value = * (const unsigned short *) *pointer;
            *pointer += sizeof(short);
            *length -= sizeof(short);
            break;

        case 32:
            if (signedp)
                value = * (const long *) *pointer;
            else
                value = * (const unsigned long *) *pointer & 0xffffffff;
            *pointer += sizeof(long);
            *length -= sizeof(long);
            break;

        default:
            return -1;
    }

    return value;
}

int
Extract_Len_String (const char **pointer, int *length, int size, const char **string)
{
    int ret_len = 0;
    int len;
    const char *s;

    if (size != 8) {
        error (msg_prop_char_size, 's', 8);
        return -1;
    }

    s = *pointer;
    len = *length;

    *string = s;

    for ( ;; ) {
        ret_len++;
        len--;

        if ( *s++ == '\0' || len == 0 )
            break;
    }

    *pointer = s;
    *length = len;
    return ret_len;
}

int
Extract_Icon (const char **pointer, int *length, int size, const char **icon)
{
    int len;

    if (size != 32) {
        error (msg_prop_char_size, 'o', 32);
        return -1;
    }

    len = *length;
    *icon = *pointer;
    *length = 0;
    return len;
}

int
Thunk_Show_Prop_Break_Down_Property (Thunks *thunks, const char *pointer, int length, Atom type, FormatAtom *fa, int size)
{
    int i;
    char format_c;
    long value;
    const char *extra = NULL;
    Atom encoding = None;

    if ( Thunks_Create (thunks, 0, sizeof (ThunkShowProp)) == NULL )
        return False;

    for ( i = 0; length >= size / 8; i++ ) {
        format_c = Format_Atom_Get_Char (fa, i);
        
        switch ( format_c ) {
            case 's':
            case 'u':
                value = Extract_Len_String (&pointer, &length, size, &extra);
                break;

            case 't':
                encoding = type;
                value = Extract_Len_String (&pointer, &length, size, &extra);
                break;

            case 'o':
                value = Extract_Icon (&pointer, &length, size, &extra);
                break;

            default:
                value = Extract_Value (&pointer, &length, size, format_c == 'i');
                break;
        }

        if ( value == -1 )
            return False;

        if ( !Thunks_Show_Prop_Add (thunks, value, extra, encoding) )
            return False;
    }

    return True;
}
