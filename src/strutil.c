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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "strutil.h"
#include "verbose.h"


char *
s_set_zero (char *s)
{
    *s++ = '0';
    *s = '\0';
    return s;
}

/* e points to s + strlen (s) ~ *e should be '\0' */
void
s_reverse_end (char *s, char *e)
{
    char c;

    while ( s < --e ) {
        /* swap characters */
        c = *s;
        *s++ = *e;
        *e = c;
    }
}

void
s_reverse (char *s)
{
    unsigned int len;

    len = strlen (s);
    s_reverse_end (s, s + len);
}

char *
s_uint (char *s, unsigned int val)
{
    char *e;

    if ( val == 0 )
        return s_set_zero (s);

    /* process individual digits */
    e = s;
    while ( val != 0 ) {
        *e++ = '0' + val % 10;
        val /= 10;
    }
    s_reverse_end (s, e);

    *e = '\0';
    return e;
}

char *
s_int (char *s, int val)
{
    if ( val < 0 ) {
        *s++ = '-';
        val = -val;
    }
    return s_uint (s, val);
}

char *
s_dup (const char *s)
{
    s = strdup (s);
    if ( s == NULL ) {
        error (msg_out_of_memory);
        return NULL;
    }
    return (char *) s;
}

const char *
s_skip_digits (const char *s)
{
    while ( isdigit (*s) )
        s++;

    return s;
}

const char *
s_parse_uint (const char *s, int *value)
{
    if ( isdigit (*s) ) {
        *value = atoi (s);
        return s_skip_digits (s + 1);  /* s[0] is a digit */
    }
    
    *value = -1;
    return s;
}

char *
s_hex (char *s, unsigned int val)
{
    int mod;
    char *e;

    *s++ = '0';
    *s++ = 'x';

    if ( val == 0 )
        return s_set_zero (s);

    /* process individual digits */
    e = s;
    do {
        mod = val % 16;
        *e++ = mod < 10 ? '0' + mod : 'W' + mod;  /* ~ 'a' + mod - 10 */
        val /= 16;
    } while ( val != 0 );
    s_reverse_end (s, e);

    *e = '\0';
    return e;
}

char *
s_append_chars (char *s, char c, unsigned int count)
{
    while ( count != 0 ) {
        *s++ = c;
        count--;
    }
    return s;
}


char *
s_append_strings (char *s, const char *a, unsigned int count)
{
    while ( count != 0 ) {
        s += sprintf (s, "%s", a);
        count--;
    }
    return s;
}

