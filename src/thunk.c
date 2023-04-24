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
#include <string.h>

#include "thunk.h"
#include "verbose.h"
#include "strutil.h"
#include "xprop.h"
#include "dsimple.h"


#define THUNK_INIT_SIZE  8


void
Thunks_Init (Thunks *thunks)
{
    thunks->items = NULL;
    thunks->nelements = 0;
}

void
Thunks_Free (Thunks *thunks)
{
    free (thunks->items);
}

unsigned char *
Thunks_Create (Thunks *thunks, unsigned int init_size, unsigned int element_size)
{
    unsigned char *items;

    if ( init_size == 0 )
        init_size = THUNK_INIT_SIZE;

    items = x_malloc (init_size * element_size);
    if ( items == NULL )
        return NULL;

    thunks->items = items;
    thunks->alloc_size = init_size;
    thunks->element_size = element_size;
    thunks->nelements = 0;

    return items;
}

unsigned char *
Thunks_Double_Size (Thunks *thunks, unsigned int min_size)
{
    unsigned char *new_items;
    unsigned int double_size;

    /* double buffer size */
    double_size = thunks->alloc_size << 1;
    if ( double_size < min_size )  /* min_size can be 0: then the condition is ignored (double_size > 0) */
        double_size = min_size;
    
    new_items = x_realloc (thunks->items, Thunks_Size_To_Bytes (thunks, double_size));
    if ( new_items == NULL )
        /* the buffer thunks->items is still valid */
        return NULL;

    /* update the buffer */
    thunks->items = new_items;
    thunks->alloc_size = double_size;
    
    return new_items;
}

char *
Thunks_String_Double_Size (Thunks *thunks, unsigned int min_size)
{
    unsigned int prev_size, remain;
    unsigned char *new_items;

    /* we will double the buffer so let's check the half size */
    prev_size = thunks->alloc_size;
    if ( prev_size > (MAX_STR >> 1) || min_size > MAX_STR ) {
        error ("reached maximum for string length");
        return NULL;
    }

    new_items = Thunks_Double_Size (thunks, min_size);
    if ( new_items == NULL )
        return NULL;

    /* offset */
    remain = thunks->nelements;
    thunks->nelements += thunks->alloc_size - prev_size;
    return (char *) new_items + prev_size - remain;
}
 
unsigned char *
Thunks_Alloc_Next (Thunks *thunks)
{
    unsigned char *items;
    
    items = thunks->nelements == thunks->alloc_size ? Thunks_Double_Size (thunks, 0) : thunks->items;
    items += Thunks_Size_To_Bytes (thunks, thunks->nelements);

    thunks->nelements++;
    return items;
}


/*
 * Thunk for remove prop
 */

ThunkRemoveProp *
Thunks_Remove_Prop_Add (Thunks *thunks, const char *name)
{
    ThunkRemoveProp *t = (ThunkRemoveProp *) Thunks_Alloc_Next (thunks);
    if ( t == NULL )
        return NULL;

    t->name = name;
    return t;
}

ThunkRemoveProp *
Thunks_Remove_Prop_Init_And_Add (Thunks *thunks, const char *name)
{
    if ( thunks->items == NULL &&
         Thunks_Create (thunks, 0, sizeof (ThunkRemoveProp)) == NULL )
        return NULL;

    return Thunks_Remove_Prop_Add (thunks, name);
}

ThunkSetProp * 
Thunks_Set_Prop_Add (Thunks *thunks, const char *arg)
{
    char *find;

    ThunkSetProp *t = (ThunkSetProp *) Thunks_Alloc_Next (thunks);
    if ( t == NULL )
        return NULL;

    /* name */
    t->name = arg;

    /* value */
    find = strchr (arg, ':');
    if (find == NULL)
        t->value = "";
    else {
        *find++ = '\0';
        t->value = find;
    }
    return t;
}

ThunkSetProp *
Thunks_Set_Prop_Init_And_Add (Thunks *thunks, const char *arg)
{
    if ( thunks->items == NULL &&
         Thunks_Create (thunks, 0, sizeof (ThunkSetProp)) == NULL )
        return NULL;

    return Thunks_Set_Prop_Add (thunks, arg);
}

/*
 * Property thunks
 */

void
Thunks_Property_Free (Thunks *thunks)
{
    ThunkProperty *t = (ThunkProperty *) thunks->items;
    int size = thunks->nelements;

    while ( size-- != 0 ) {
        /* only delimiter is duplicated */
        free ((char *) t->fmt.d.delim);
        t++;
    }
   
    free (thunks->items);
}

ThunkProperty *
Thunks_Property_Find_Atom (Thunks *thunks, Atom atom)
{
    int i;
    ThunkProperty *t = (ThunkProperty *) thunks->items;

    for (i = thunks->nelements; i != 0; i--, t++) {
        if ( t->atom == atom )
            return t;
    }
    return NULL;
}

/*
 * Show_Prop thunks
 */

ThunkShowProp * 
Thunks_Show_Prop_Add (Thunks *thunks, int value, const char *extra, Atom encoding)
{
    ThunkShowProp *t = (ThunkShowProp *) Thunks_Alloc_Next (thunks);
    if ( t == NULL )
        return NULL;
    
    t->value = value;
    t->extra = extra;
    t->encoding = encoding;

    return t;
}

ThunkMapping * 
Thunks_Mapping_Add (Thunks *thunks, Atom atom, Format *fmt)
{
    ThunkMapping *t = (ThunkMapping *) Thunks_Alloc_Next (thunks);
    if ( t == NULL )
        return NULL;

    t->atom = atom;
    Format_Copy (&t->fmt, fmt);
    return t;
}

ThunkMapping * 
Thunks_Mapping_Find_Atom (Thunks *thunks, Atom atom)
{
    int i;
    ThunkMapping *t = (ThunkMapping *) thunks->items;

    for (i = thunks->nelements - 1, t += i; i >= 0; i--, t--) {
        if ( t->atom == atom )
            return t;
    }
    return NULL;
}

void
Thunks_Mapping_Lookup_Formats (Thunks *thunks, Atom atom, Format *fmt)
{
    ThunkMapping *t;

    t = Thunks_Mapping_Find_Atom (thunks, atom);
    if ( t == NULL )
        return;

    /* format */
    if ( fmt->a.format == NULL && t->fmt.a.format != NULL ) {
   	    Format_Atom_Copy (&fmt->a, &t->fmt.a);
    }

    /* dformat */
	if ( fmt->d.format == NULL )
        fmt->d.format = t->fmt.d.format;

    /* delim */
    if ( fmt->d.delim == NULL )
        fmt->d.delim = t->fmt.d.delim;
}

void
Thunks_Mapping_Free (Thunks *thunks, int free_struct)
{
    ThunkMapping *t = (ThunkMapping *) thunks->items;
    int size = thunks->nelements;

    while ( size-- != 0 ) {
        if ( free_struct )
            Format_Free (&t->fmt);
        else
            /* only delimiter */
            free ((char *) t->fmt.d.delim);

        t++;
    }

    free (thunks->items);
}

ThunkMapping *
Thunks_Mapping_Init_And_Add (Thunks *thunks, Atom atom, Format *fmt)
{
    if ( thunks->items == NULL &&
         Thunks_Create (thunks, 0, sizeof (ThunkMapping)) == NULL )
        return NULL;

    return Thunks_Mapping_Add (thunks, atom, fmt);
}

int
Thunks_Mapping_Read (Thunks *thunks, FILE *stream)
{
    char szformat [128];
    char szname [1024];
    char *s_new;
    int count;
    char c;
    ThunkMapping *tm, *tm_new;

    tm = (ThunkMapping *) Thunks_Create (thunks, 0, sizeof (ThunkMapping));
    if ( tm == NULL )
        return False;

    while ( (count = fscanf (stream, " %990s %90s ", szname, szformat)) != EOF ) {
        if ( count != 2 ) {
            error ("bad format file format");
            return False;
        }

        /* we need to clear the structure because it can be
         * filled w/ previous fields */
        Format_Clear (&tm->fmt);

        /* property */
        tm->atom = Parse_Atom (szname, False);

        /* format */
        s_new = s_dup (szformat);
        if ( s_new == NULL )
            return False;

        if ( !Format_Atom_Parse (&tm->fmt.a, s_new) )
            goto quit;

        /* dformat */
        Read_White_Space (stream);
        c = getc (stream);
        if ( c == (int) '\'' ) {
            s_new = Read_Quoted (stream);
            if ( s_new == NULL )
                goto quit;

            s_new = s_dup (s_new);
            if ( s_new == NULL )
                goto quit;

            if ( !Format_Display_Parse (&tm->fmt.d, s_new) )
                goto quit;
        } else
            ungetc (c, stream);

        /* increment $nelements and check allocated size */
        if ( ++thunks->nelements != thunks->alloc_size )
            tm++;
        else {
            tm_new = (ThunkMapping *) Thunks_Double_Size (thunks, 0);
            if ( tm_new == NULL )
                goto quit;

            tm = tm_new + thunks->nelements;
        }
    }

    return True;

quit:
    
    free (s_new);
    Format_Free (&tm->fmt);

    return False;
}

int
Thunks_Mapping_Format_Parse (Thunks *thunks, const char *arg_name, char *arg_value)
{
    Format fmt;
    Atom atom;

    if ( !Args_Format_Parse_Name_Format (&fmt, arg_name, arg_value, FormatDisplayOptional) )
        return False;

    /* atom */
    atom = Parse_Atom (arg_value, False);

    /* add new mapping */
    if ( !Thunks_Mapping_Init_And_Add (thunks, atom, &fmt) ) {
        /* $delim is duplicated! */
        free ((char *) fmt.d.delim);
        return False;
    }

    return True;
}

int
Thunks_Mapping_Read_File (Thunks *thunks, const char *name)
{
    FILE *stream;
    int ret;

    stream = fopen(name, "r");
    if ( stream == NULL ) {
        error ("unable to open file %s for reading", name);
        return False;
    }

    ret = Thunks_Mapping_Read (thunks, stream);
    fclose(stream);
    return ret;
}

int
Thunks_Mapping_PropertyRec_Map (Thunks *thunks, PropertyRec *ptable, unsigned int size)
{
    ThunkMapping *tm, *tm_new;

    tm = (ThunkMapping *) Thunks_Create (thunks, 0, sizeof (ThunkMapping));
    if ( tm == NULL )
        return False;
           
    for ( ; size != 0; size--, ptable++ ) {
        /* atom */
        if ( ptable->atom == None ) {
            ptable->atom = XInternAtom (d_dpy, ptable->name, True);
            if ( ptable->atom == None )
                continue;
        }

        tm->atom = ptable->atom;
                      
        /* parse format: internal formats should be valid so no checking */
        Format_Atom_Parse (&tm->fmt.a, ptable->format);
                       
        /* parse dformat: we duplicate delimiter so let's check the result */
        if ( ptable->dformat == NULL )
            Format_Display_Clear (&tm->fmt.d);
        else if ( !Format_Display_Parse (&tm->fmt.d, ptable->dformat) )
            return False;

        /* increment $nelements and check allocated size */
        if ( ++thunks->nelements != thunks->alloc_size )
            tm++;
        else {
            tm_new = (ThunkMapping *) Thunks_Double_Size (thunks, 0);
            if ( tm_new == NULL ) {
                /* don't forget to release memory */
                free ((char *) tm->fmt.d.delim);
                return False;
            }
            tm = tm_new + thunks->nelements;
        }
    }

    return True;
}
