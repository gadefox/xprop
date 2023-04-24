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

#ifndef _FORMAT_H_
#define _FORMAT_H_

typedef struct {
    unsigned int size;
    unsigned int imax;
    const char *format;
} FormatAtom;

typedef struct {
    const char *format;
    const char *delim;
} FormatDisplay;

typedef struct {
    FormatAtom a;
    FormatDisplay d;
} Format;


extern const char def_format[];
extern const char def_dformat[];
extern const char def_delim[];

extern const char missing_dformat[];
 
extern Format fmt_default;


/*
 * Default
 */

int Format_Print_Default (void);


/*
 * Format atom
 */

void Format_Atom_Set_Default (FormatAtom *fa);
char Format_Atom_Get_Char (FormatAtom *fa, unsigned int i);
int Format_Atom_Is (const char *format);
int Format_Atom_Parse (FormatAtom *fa, const char *format);
int Format_Atom_Dup (FormatAtom *dst, FormatAtom *src);
void Format_Atom_Copy (FormatAtom *dst, FormatAtom *src);
void Format_Atom_Clear (FormatAtom *fa);
void Format_Atom_Free (FormatAtom *fa);


/*
 * Format display
 */

int Format_Display_Copy_Delim (const char *format, char *delim, unsigned int size);
void Format_Display_Set_Default (FormatDisplay *fd);
const char * Format_Display_Skip_Delim (const char *format);
int Format_Display_Parse_Delim (const char *format, char *delim, unsigned int size);
int Format_Display_Parse (FormatDisplay *fd, const char *format);
void Format_Display_Copy (FormatDisplay *dst, FormatDisplay *src);
int Format_Display_Dup (FormatDisplay *dst, FormatDisplay *src);
void Format_Display_Clear (FormatDisplay *fd);
void Format_Display_Free (FormatDisplay *fd);


/*
 * Format:
 *
 * all possibilities:
 *   format: null;     dformat null
 *   format: not null; dformat null
 *   format: not null; dformat not null
 */

void Format_Copy (Format *dst, Format *src);
int Format_Dup (Format *dst, Format *src);
void Format_Clear (Format *fmt);
void Format_Free (Format *fmt);
void Format_Set_Default (Format *fmt);
void Format_Apply_Default (Format *fmt);


#endif  /* _FORMAT_H_ */
