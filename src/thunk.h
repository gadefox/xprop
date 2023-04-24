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

#ifndef _THUNK_H_
#define _THUNK_H_

#include <X11/Xlib.h>
#include <stdio.h>

#include "format.h"


typedef struct {
    const char *name;
    Atom atom;
    const char *format;
    const char *dformat;
} PropertyRec;


/*
 * Thunks
 */

#define MAX_STR  (1 << 19)  /* 512 KB */

#define Thunks_Size_To_Bytes(t, i)  ((t)->element_size * (i))
#define Thunks_Get_Item(t, i)       ((t)->items + Thunks_Size_To_Bytes (t, i))


/* thunk for removing properties */
typedef struct {
    const char *name;
} ThunkRemoveProp;

/* thunk for setting properties */
typedef struct {
    const char *name;
    const char *value;
} ThunkSetProp;

/* property mappings */
typedef struct {
    Atom atom;
    Format fmt;
} ThunkMapping;

/* user properties */
typedef struct {
    Atom atom;
    const char *name;
    Format fmt;
} ThunkProperty;

typedef struct {
    int value;
    Atom encoding;
    const char *extra;
} ThunkShowProp;

typedef struct {
    unsigned char *items;       /* elements */
    unsigned int alloc_size;    /* allocated elements */
    unsigned int element_size;  /* sizeof (<Element>) */
    unsigned int nelements;     /* elements used */
} Thunks;


void Thunks_Init (Thunks *thunks);
void Thunks_Free (Thunks *thunks);
unsigned char * Thunks_Create (Thunks *thunks, unsigned int init_size, unsigned int element_size);
unsigned char * Thunks_Double_Size (Thunks *thunks, unsigned int min_size);
char * Thunks_String_Double_Size (Thunks *thunks, unsigned int min_size);
unsigned char * Thunks_Alloc_Next (Thunks *thunks);

ThunkRemoveProp * Thunks_Remove_Prop_Add (Thunks *thunks, const char *name);
ThunkRemoveProp * Thunks_Remove_Prop_Init_And_Add (Thunks *thunks, const char *name);

ThunkSetProp * Thunks_Set_Prop_Add (Thunks *thunks, const char *arg);
ThunkSetProp * Thunks_Set_Prop_Init_And_Add (Thunks *thunks, const char *arg);

void Thunks_Property_Free (Thunks *thunks);
ThunkProperty * Thunks_Property_Find_Atom (Thunks *thunks, Atom atom);

ThunkShowProp * Thunks_Show_Prop_Add (Thunks *thunks, int value, const char *extra, Atom encoding);

ThunkMapping * Thunks_Mapping_Add (Thunks *thunks, Atom atom, Format *fmt);
ThunkMapping * Thunks_Mapping_Find_Atom (Thunks *thunks, Atom atom);
ThunkMapping * Thunks_Mapping_Init_And_Add (Thunks *thunks, Atom atom, Format *fmt);
void Thunks_Mapping_Free (Thunks *thunks, int free_struct);

void Thunks_Mapping_Lookup_Formats (Thunks *thunks, Atom atom, Format *fmt);

int Thunks_Mapping_Read (Thunks *thunks, FILE *stream);
int Thunks_Mapping_Format_Parse (Thunks *thunks, const char *arg_name, char *arg_value);
int Thunks_Mapping_Read_File (Thunks *thunks, const char *name);

int Thunks_Mapping_PropertyRec_Map (Thunks *thunks, PropertyRec *ptable, unsigned int size);


#endif  /* _THUNK_H_ */
