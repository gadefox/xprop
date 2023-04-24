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

#ifndef _MISC_H_
#define _MISC_H_

#include "args.h"


#ifndef min
#define min(a,b)  ((a) < (b) ? (a) : (b))
#endif

/* isprint() in "C" locale */
#define c_isprint(c)  ((c) >= 0x20 && (c) < 0x7f)
#define countof(a)    (sizeof (a) / sizeof ((a) [0]))

#define MAX_NUM_SIZE  (countof ("18446744073709551615"))


typedef enum {
    PropertyWindow = (1 << 0),
    PropertyFont   = 0,
    NameUndefined  = (1 << 1),
    TypeUndefined  = (1 << 2)
} NameTypeFlags;

typedef struct {
    const char *name;
    const char *type;
    unsigned int flags;
} NameType;


const char * Get_Prog_Name (const char *name);

int is_truecolor_term (void);
unsigned int get_term_width (void);

void * x_malloc (unsigned int size);
void * x_realloc (void *p, unsigned int size);

int Read_Char (FILE *stream);
void Read_White_Space (FILE *stream);
char * Read_Quoted (FILE *stream);
const char * Scan_Octal (const char *s, int *value);

Atom Parse_Atom (const char *name, int only_if_exists);
char * Get_Undefined_Atom_Name (Atom atom);

const char * PropertyRec_Find_Atom_Name (PropertyRec *ptable, unsigned int size, Atom atom);
XFontProp * Font_Property_Find_Atom (XFontProp *fprop, uint size, Atom atom);

const char * Get_Font_Property_Data_And_Type (Atom atom, long *length, Atom *type, int *size);
Atom Get_Window_Property_Type (Atom atom);
const char * Get_Window_Property_Data_And_Type (Atom atom, long *length, Atom *type, int *size);

void NameType_Free (NameType *np);
void NameType_Clear (NameType *np);
int NameType_Dup (NameType *dst, NameType *src);

void Remove_Property (Window w, const char *propname);
const unsigned char * Set_Property_Get_Data (const char *value, int size, int *elements);
int Set_Property (RunParams *rp, Window w, const char *propname, const char *value);


#endif  /* _MISC_H_ */
