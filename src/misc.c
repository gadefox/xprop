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

#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "misc.h"
#include "verbose.h"
#include "strutil.h"
#include "dsimple.h"
#include "xprop.h"
#include "format.h"
#include "print.h"


#define MAX_ELEMENTS  64


/*
 * Misc. routines
 */

const char *
Get_Prog_Name (const char *name)
{
    char *p;

    p = strrchr (name, '/');
    if ( p != NULL )
        return p + 1;

    return name;
}

void *
x_malloc (unsigned int size)
{
    void *p = malloc (size);
    if ( p == NULL ) {
        error (msg_out_of_memory);
        return NULL;
    }
    return p;
}

void *
x_realloc (void *p, unsigned int size)
{
    p = realloc (p, size);
    if ( p == NULL ) {
        error (msg_out_of_memory);
        return NULL;
    }
    return p;
}

int
Read_Char (FILE *stream)
{
    int c;

    c = getc (stream);
    if ( c == EOF ) {
    	error ("format: unexpected EOF");
        return -1;
    }

    return c;
}

void
Read_White_Space (FILE *stream)
{
    char c;

    for ( ;; ) {
        c = getc (stream);
        if ( c != ' ' && c != '\n' && c != '\t')
            break;
    }

    ungetc (c, stream);
}

char *
Read_Quoted (FILE *stream)
{
    Large_Buffer_Variables;
    unsigned int c;

    Large_Buffer_Init ();
    for ( ;; )
      {
    	c = Read_Char (stream);
        if ( c == -1 )
            return NULL;

        if ( c == '\'' )
	        break;

        if ( c == (int) '\\' ) {
    	    c = Read_Char (stream);
            if ( c == -1 )
                return NULL;

            if ( c == '\n' )
                continue;
            
            Large_Buffer_Put_Char ('\\');    
    	}
      
        Large_Buffer_Put_Char (c);
	
     	if ( !Large_Buffer_Is_Enough (3) ) {  /* strlen ("\\<c>\0") */
            s_large_buffer = Thunks_String_Double_Size (&xp_large_buffer, 0);
            if ( s_large_buffer == NULL )
                return NULL;
        }
      }

    *s_large_buffer = '\0';
    return (char *) xp_large_buffer.items;
}

const char *
Scan_Octal (const char *s, int *value)
{
    if ( sscanf (s, "%o", (uint *) value) != 1 ) {
    	error ("invalid octal #: %s", s);
        return NULL;
    }

    return s_skip_digits (s);
}

Atom
Parse_Atom (const char *name, int only_if_exists)
{
    /* may return None = 0 */
    return XInternAtom (d_dpy, name, only_if_exists);
}


/*
 * PropertyRec
 */

const char *
PropertyRec_Find_Atom_Name (PropertyRec *ptable, unsigned int size, Atom atom)
{
    for ( ; size != 0; size--, ptable++ ) {
        if ( ptable->atom == atom )
            return ptable->name;
    }
    return NULL;
}

XFontProp *
Font_Property_Find_Atom (XFontProp *fprop, uint size, Atom atom)
{
    for ( ; size != 0; size--, fprop++ ) {
        if ( fprop->name == atom )
            return fprop;
    }
    return NULL; 
}


const char *
Get_Font_Property_Data_And_Type (Atom atom, long *length, Atom *type, int *size)
{
    XFontProp *fprop;

    *type = None;

    fprop = Font_Property_Find_Atom (xp_font->properties, xp_font->n_properties, atom);
    if ( fprop == NULL ) {
        *size = 0;
        return NULL;
    }

    xp_font_prop = fprop->card32;
    *length = sizeof (long);
    *size = 32;
    return (const char *) &xp_font_prop;
}

Atom 
Get_Window_Property_Type (Atom atom)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop = NULL;
    int status;

    status = XGetWindowProperty (d_dpy, xp_target_win, atom, 0, 0, False,
           AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);

    return status != Success ? actual_type : None;
}

const char *
Get_Window_Property_Data_And_Type (Atom atom, long *length, Atom *type, int *size)
{
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long nbytes;
    unsigned long bytes_after;
    unsigned char *prop = NULL;
    int status;

    status = XGetWindowProperty (d_dpy, xp_target_win, atom, 0, (a_max_len + 3) >> 2, False,
           AnyPropertyType, &actual_type, &actual_format, &nitems, &bytes_after, &prop);

    if ( status != Success ) {
        if ( status == BadWindow )
            error ("window: id # 0x%lx does not exists", xp_target_win);
        else
            error ("XGetWindowProperty failed");
        return NULL;
    }

    if (actual_format == 32)
        nbytes = sizeof(long);
    else if (actual_format == 16)
        nbytes = sizeof(short);
    else if (actual_format == 8)
        nbytes = 1;
    else if (actual_format == 0)
        nbytes = 0;
    else
        return NULL;

    *length = min(nitems * nbytes, a_max_len);
    *type = actual_type;
    *size = actual_format;
    return (const char *)prop;
}

void
NameType_Clear (NameType *np)
{
    np->name = NULL;
    np->type = NULL;
}

void
NameType_Free (NameType *np)
{
    free ((char *) np->name);
    free ((char *) np->type);
    NameType_Clear (np);
}

int
NameType_Dup (NameType *dst, NameType *src)
{
    /* prop name */
    dst->name = s_dup (src->name);
    if ( dst->name == NULL )
        return False;

    /* prop type */
    if ( src->type == NULL )
        dst->type = NULL;
    else {
        dst->type = s_dup (src->type);
        if ( dst->type == NULL ) {
            NameType_Free (dst);
            return False;
        }
    }

    /* flags */
    dst->flags = src->flags;
    return True;
}

void
Remove_Property (Window w, const char *propname)
{
    Atom id = XInternAtom (d_dpy, propname, True);
    if (id == None)
        error (msg_prop_not_found, propname);
    else
        XDeleteProperty (d_dpy, w, id);
}

const unsigned char *
Set_Property_Get_Data (const char *value, int size, int *elements)
{
    static unsigned char  data8  [MAX_ELEMENTS];
    static unsigned short data16 [MAX_ELEMENTS];
    static unsigned long  data32 [MAX_ELEMENTS];

    unsigned long intvalue;
    char *dup;
    char *tmp;
    int count;
    unsigned char *p8;
    unsigned short *p16;
    unsigned long *p32;
    const unsigned char *data = NULL;

    dup = s_dup (value);
    if ( dup == NULL )
        return NULL;

    tmp = strtok (dup, ",");
    intvalue = strtoul (tmp, NULL, 0);
    switch (size) {
        case 8:
            p8 = data8;
            *p8++ = intvalue;
            data = (const unsigned char *) data8;
            break;

        case 16:
            p16 = data16;
            *p16++ = intvalue;
            data =(const unsigned char *) data16;
            break;

        case 32:
            p32 = data32;
            *p32++ = intvalue;
            data = (const unsigned char *) data32;
            break;
    }

    count = 1;
    tmp = strtok (NULL, ",");

    while (tmp != NULL) {
        intvalue = strtoul (tmp, NULL, 0);
        switch (size) {
            case 8:
                *p8++ = intvalue;
                break;

            case 16:    
                *p16++ = intvalue;
                break;

            case 32:    
                *p32 = intvalue;
                break;
        }
        if ( ++count == MAX_ELEMENTS ) {
            warn ("maximum number of elements reached (%d); list truncated", MAX_ELEMENTS);
            break;
        }
        tmp = strtok (NULL, ",");
    }

    /* free allocated buffer */
    free (dup);

    *elements = count;
    return data;
}

int
Set_Property (RunParams *rp, Window w, const char *propname, const char *value)
{
    Atom atom, type;
    char format;
    const unsigned char *data;
    int nelements;
    Format fmt;

    /* property name and type */
    atom = Parse_Atom (propname, False);
    type = Get_Window_Property_Type (atom);

    /* check format */
    fmt.a.format = NULL;

    Run_Params_Lookup_Formats (rp, atom, &fmt);
    Run_Params_Lookup_Formats (rp, type, &fmt);

    if ( fmt.a.format == NULL ) {
        error ("%s: unsupported conversion", propname);
        return False;
    }

    format = Format_Atom_Get_Char (&fmt.a, 0);
    switch (format) {
        case 's':
            if ( fmt.a.size != 8 ) {
                error (msg_prop_char_size, 's', 8);
                return False;
            }

            type = XA_STRING;
            data = (const unsigned char *) value;
            nelements = strlen(value);
            break;

        case 'u':
            if ( fmt.a.size != 8 ) {
                error (msg_prop_char_size, 'u', 8);
                return False;
            }

            type = XInternAtom(d_dpy, "UTF8_STRING", False);
            data = (const unsigned char *) value;
            nelements = strlen(value);
            break;

        case 't':
            {
                XTextProperty textprop;
                if ( fmt.a.size != 8 ) {
                    error (msg_prop_char_size, 't', 8);
                    return False;
                }

                if (XmbTextListToTextProperty (d_dpy, (char **) &value, 1, XStdICCTextStyle, &textprop) != Success) {
                    error ("cannot convert %s argument to STRING or COMPOUND_TEXT", propname);
                    return True;
                }

                type = textprop.encoding;
                data = textprop.value;
                nelements = textprop.nitems;
                break;
            }

        case 'x':
        case 'c':
            data = Set_Property_Get_Data (value, fmt.a.size, &nelements);
            if ( data == NULL )
                return False;

            type = XA_CARDINAL;
            break;

        case 'i':
            data = Set_Property_Get_Data (value, fmt.a.size, &nelements);
            if ( data == NULL )
                return False;

            type = XA_INTEGER;
            break;

        case 'b':
            {
                unsigned long boolvalue, data32;
                unsigned char data8;
                unsigned short data16;

                if ( strcmp (value, true_name) == 0 )
                    boolvalue = True;
                else if ( strcmp (value, false_name) == 0)
                    boolvalue = False;
                else {
                    warn("cannot convert %s argument to Bool", propname);
                    return True;
                }

                type = XA_INTEGER;
                switch ( fmt.a.size ) {
                    case 8:
                        data8 = boolvalue;
                        data = (const unsigned char *) &data8;
                        break;

                    case 16:
                        data16 = boolvalue;
                        data = (const unsigned char *) &data16;
                        break;

                    case 32:
                    default:
                        data32 = boolvalue;
                        data = (const unsigned char *) &data32;
                        break;
                }

                nelements = 1;
                break;
            }

        case 'a':
            {
                static Atom avalue;

                avalue = Parse_Atom(value, False);
                type = XA_ATOM;
                data = (const unsigned char *) &avalue;
                nelements = 1;
                break;
            }

        default:  /* 'm' NYI */
            return True;
    }

    XChangeProperty (d_dpy, xp_target_win, atom, type, fmt.a.size, PropModeReplace, data, nelements);
    return True;
}

char *
Get_Undefined_Atom_Name (Atom atom)
{
    snprintf ((char *) xp_large_buffer.items, xp_large_buffer.alloc_size, "undefined atom # 0x%lx", atom);
    return (char *) xp_large_buffer.items;
}

int
is_truecolor_term (void)
{
    char *colorterm = getenv( "COLORTERM" );
    if ( colorterm == NULL )
        return False;

    return strcmp (colorterm, "truecolor") == 0;
}

unsigned int
get_term_width (void)
{
#ifdef TIOCGWINSZ
    struct winsize ws;
    ws.ws_col = 0;

    if ( ioctl (STDIN_FILENO, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0)
        return ws.ws_col;
#endif /* TIOCGWINSZ */

    return MAX_ICON_WIDTH;
}

