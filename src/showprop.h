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

#ifndef _SHOW_PROP_H_
#define _SHOW_PROP_H_

#include "thunk.h"
#include "misc.h"


const char * Skip_Past_Right_Paren (const char *string);

int Thunk_Show_Prop_Print (ThunkShowProp *t, char format_char, const char *delim);
int Thunk_Show_Prop_Print_I (Thunks *thunks, Format *fmt, unsigned int i);
int Thunk_Show_Prop_Mask_Word (Thunks *thunks, FormatAtom *fa);
int Thunk_Show_Prop_Mask_Bit_I (Thunks *thunks, FormatAtom *fa, unsigned int i);

const char * Handle_Backslash (const char *dformat);
const char * Handle_Dollar (const char *dformat, NameType *nt, Thunks *thunks, Format *fmt);
const char * Handle_Question_Mark (const char *dformat, Thunks *thunks, FormatAtom *fa, int *question);

const char * Thunk_Show_Prop_Scan_Term (const char *s, Thunks *thunks, FormatAtom *fa, int *value);
const char * Thunk_Show_Prop_Scan_Exp (const char *string, Thunks *thunks, FormatAtom *fa, int *value);

int Thunk_Show_Prop_Display_Property (NameType *nt, Thunks *thunks, Format *fmt);

int Extract_Value (const char **pointer, int *length, int size, int signedp);
int Extract_Len_String (const char **pointer, int *length, int size, const char **string);
int Extract_Icon (const char **pointer, int *length, int size, const char **icon);

int Thunk_Show_Prop_Break_Down_Property (Thunks *thunks, const char *pointer, int length, Atom type, FormatAtom *fa, int size);


#endif  /* _SHOW_PROP_H_ */
