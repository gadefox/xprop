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

#ifndef _PRINT_H_
#define _PRINT_H_

#include "verbose.h"
#include "misc.h"


#define MAX_ICON_WIDTH   144
#define MAX_ICON_HEIGHT  144

#define ColorValue      VerboseCyan
#define ColorUndefined  VerboseYellow
#define ColorProperty   VerboseBlue
#define ColorFont       VerboseMagenda
#define ColorKeyword    VerboseWhite


/*
 * Validate a string as UTF-8 encoded according to RFC 3629
 *
 * Simply, a unicode code point (up to 21-bits long) is encoded as follows:
 *
 *    Char. number range  |        UTF-8 octet sequence
 *       (hexadecimal)    |              (binary)
 *    --------------------+---------------------------------------------
 *    0000 0000-0000 007F | 0xxxxxxx
 *    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
 *    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
 *    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 * Validation is done left-to-right, and an error condition, if any, refers to
 * only the left-most problem in the string.
 *
 * Return values:
 *   UTF8_VALID: Valid UTF-8 encoded string
 *   UTF8_OVERLONG: Using more bytes than needed for a code point
 *   UTF8_SHORT_TAIL: Not enough bytes in a multi-byte sequence
 *   UTF8_LONG_TAIL: Too many bytes in a multi-byte sequence
 *   UTF8_FORBIDDEN_VALUE: Forbidden prefix or code point outside 0x10FFFF
*/

typedef enum {
    UTF8_Valid,
    UTF8_ForbiddenValue,
    UTF8_Overlong,
    UTF8_ShortTail,
    UTF8_LongTail
} UTF8_Status;


extern const char true_name[];
extern const char false_name[];


UTF8_Status UTF8_Is_Valid (const char *s, unsigned int len);
void UTF8_Status_Print_Error (const char *error);
int UTF8_Is_Locale (void);


void Print (const char *value, VerboseColor color);
void Print_Unsigned (unsigned int value, VerboseColor color);
void Print_Signed (int value, VerboseColor color);
void Print_Hex (int wrd);
void Print_Atom (Atom atom);
void Print_Mask_Word (int wrd, const char *delim);
const char * Bool_To_String (int value);
void Print_Bool (int value);
int Format_String (const char *s, int unicode);
int Print_Len_String (const char *s, int len, int unicode);
int Print_String (const char *s, int unicode);
const unsigned long * Print_Icon (const unsigned long *icon);
int Print_Icons (const unsigned long *icon, int bytes);
int Print_Len_Text (const char *s, int len, Atom encoding);
int Print_Len_Unicode (const char *s, int len);
int Print_Property_Rec (PropertyRec *p, VerboseColor c);
int Print_Property_Recs (const char *name, PropertyRec *p, unsigned int size, VerboseColor color);


#endif  /* _PRINT_H_ */
