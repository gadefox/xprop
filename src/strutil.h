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

#ifndef _STRING_H_
#define _STRING_H_

#define MAX_HEX_SIZE  (64 / 8 + 3)  /* 64bit / 8 + strlen("0x\0") */

#define s_append_spaces(s, c)   (s_append_chars ((s), ' ', (c)))


char * s_dup (const char *s);

char * s_set_zero (char *s);
void s_reverse_end (char *s, char *e);
void s_reverse (char *s);

char * s_uint (char *s, unsigned int val);
char * s_int (char *s, int val);
char * s_hex (char *s, unsigned int val);

const char * s_skip_digits (const char *s);
const char * s_parse_uint (const char *s, int *value);

char * s_append_chars (char *s, char c, unsigned int count);
char * s_append_strings (char *s, const char *a, unsigned int count);


#endif  /* _STRING_H_ */
