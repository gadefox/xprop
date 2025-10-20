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

#ifndef _XPROP_H_
#define _XPROP_H_

#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "misc.h"


#define Remain_Buffer_Variables     char *s_remain_buffer; unsigned int n_remain_buffer
#define Remain_Buffer_Init(s, n)    { _n_remain_buffer = (n); s_large_buffer = (s); }
#define Remain_Buffer_Put_Char(c)   { *s_remain_buffer++ = (c); n_remain_buffer--; }
#define Remain_Buffer_Offset(n)     { s_remain_buffer += (n); n_remain_buffer -= (n); }
#define Remain_Buffer_Is_Enough(n)  ( n_remain_buffer >= (n) )

#define Large_Buffer_Variables     char *s_large_buffer
#define Large_Buffer_Init()        { xp_large_buffer.nelements = xp_large_buffer.alloc_size; s_large_buffer = (char *) xp_large_buffer.items; }
#define Large_Buffer_Put_Char(c)   { *s_large_buffer++ = (c); xp_large_buffer.nelements--; }
#define Large_Buffer_Offset(n)     { s_large_buffer += (n); xp_large_buffer.nelements -= (n); }
#define Large_Buffer_Is_Enough(n)  ( xp_large_buffer.nelements >= (n) )


typedef enum {
    PropertyUndefined,
    PropertyWindowTable,
    PropertyFontTable,
    PropertySystem,
} PropertySource;

extern Window xp_target_win;
extern XFontStruct *xp_font;
extern unsigned long xp_font_prop;
extern Thunks xp_large_buffer;
extern unsigned int xp_term_width;


const char * Get_Atom_Name (Atom atom, PropertySource *src);


#endif  /* _XPROP_H_ */
