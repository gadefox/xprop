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
 *
 *
 * Modified by Rene Durk (gadefoxren@gmail.com)  12.01.22
 *
 */

#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>  /* XTextProperty */

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif
#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#ifndef HAVE_WCTYPE_H
  #define iswprint(x) isprint(x)
#endif

#include "print.h"
#include "verbose.h"
#include "strutil.h"
#include "xprop.h"
#include "misc.h"
#include "dsimple.h"


/*
 *
 * Formatting Routines: a group of routines to translate from various
 *                      values to a static read-only string useful for output.
 */

const char true_name[]  = "True";
const char false_name[] = "False";


void
Print (const char *value, VerboseColor color)
{
    if ( (a_flags & FlagRaw) || color == VerboseUndefined )
        verbose_s (value);
    else
        verbose_color (stdout, value, color);
}

void
Print_Unsigned (unsigned int value, VerboseColor color)
{
    if ( (a_flags & FlagRaw) || color == VerboseUndefined  )
        verbose_u (value);
    else
        verbose_color_uint (value, color);
}

void
Print_Signed (int value, VerboseColor color)
{
    if ( (a_flags & FlagRaw) || color == VerboseUndefined  )
        verbose_d (value);
    else
        verbose_color_int (value, color);
}

void
Print_Hex (int wrd)
{
    char szhex [MAX_HEX_SIZE];

    s_hex (szhex, wrd);
    Print (szhex, ColorValue);
}

void
Print_Atom (Atom atom)
{
    const char *name;
    PropertySource src;
    VerboseColor color;

    name = Get_Atom_Name (atom, &src);
    switch ( src ) {
        case PropertyUndefined:
            color = ColorUndefined;
            break;

        case PropertyFontTable:
            color = ColorFont;
            break;

        default:
            color = ColorProperty;
            break;
    }
    Print (name, color);
}

void
Print_Mask_Word (int wrd, const char *delim)
{
    int bit_mask, bit;
    int seen = False;

    verbose_c ('{');
    Print ("MASK: ", ColorKeyword);

    for (bit = 0, bit_mask = 1; bit <= (sizeof (int) << 3); bit++, bit_mask <<= 1) {
        if (bit_mask & wrd) {
            if (seen)
                verbose_s (delim);
            else
                seen = True;

            Print_Unsigned (bit, ColorValue);
        }
    }
    verbose_c ('}');
}

const char *
Bool_To_String (int value)
{
    if ( value )
        return true_name;

   return false_name;
}

void
Print_Bool (int value)
{
    if ( value )
        Print (true_name, VerboseGreen );
    else
        Print (false_name, VerboseRed );
}

int
Format_String (const char *s, int unicode)
{
    Large_Buffer_Variables;
    char c;

    Large_Buffer_Init ();

    /* quote */
    if ( a_flags & FlagQuote )
        Large_Buffer_Put_Char ('\"');

    for ( ;; )
      {
        c = *s++;
        if ( c == '\0')
            break;

        /* puts max 4 chars */
        switch ( c )
          {
            case '\\':
            case '\"':
                Large_Buffer_Put_Char ('\\');
                Large_Buffer_Put_Char (c);
                /* 2 chars */
                break;

            case '\n':
                Large_Buffer_Put_Char ('\\');
                Large_Buffer_Put_Char ('n');
                /* 2 chars */
                break;

            case '\t':
                Large_Buffer_Put_Char ('\\');
                Large_Buffer_Put_Char ('t');
                /* 2 chars */
                break;

            default:
                if ( c_isprint (c) ) {
                    Large_Buffer_Put_Char (c);
                    /* 1 char */
                    break;
                }

                if ( unicode && (c & 0x80) ) {
                    Large_Buffer_Put_Char (c);
                    /* 1 char */
                    break;
                }

                Large_Buffer_Put_Char ('\\');
                snprintf (s_large_buffer, 4, "%03o", (unsigned char) c);
                Large_Buffer_Offset (3);
                /* 4 chars */
                break;
          }

        if ( !Large_Buffer_Is_Enough (6) ) {  /* 4 chars (see above) + strlen ("\"\0") (see bellow) */
            s_large_buffer = Thunks_String_Double_Size (&xp_large_buffer, 0);
            if ( s_large_buffer == NULL )
                return False;
        }
      }
    /* unquote: we don't need to decrement $nelements so we're using s_remain_buffer instead of Large_Buffer_Put_Char */
    if ( a_flags & FlagQuote )
        *s_large_buffer++ = '\"';

    *s_large_buffer = '\0';
    return True;
}

int
Print_Len_String (const char *s, int len, int unicode)
{
    char *new_s;
    int ret;

    new_s = x_malloc ((len + 1) + sizeof (char));
    if ( new_s == NULL )
        return False;

    memcpy (new_s, s, len * sizeof (char));
    new_s [len] = '\0';

    ret = Format_String (new_s, unicode);
    free (new_s);
    if ( !ret )
        return False;

    /* verbose */
    Print ((char *) xp_large_buffer.items, ColorValue);
    return True;
}  

int
Print_String (const char *s, int unicode)
{
    if ( !Format_String (s, unicode) )
        return False;

    Print ((char *) xp_large_buffer.items, ColorValue);
    return True;
}  


int
UTF8_Is_Locale (void)
{
#ifdef HAVE_LANGINFO_H
    char *charmap = nl_langinfo (CODESET);

    return charmap != NULL && strcmp (charmap, "UTF-8") == 0;
#else
    return False;
#endif
}

void 
Print_Icon (const unsigned long *icon, unsigned int width, unsigned int height)
{
    int w, h, idx;
    unsigned char a, r, g, b;
    unsigned long pixel;
    unsigned long brightness;
    char *s = (char *) xp_large_buffer.items;

    for ( h = 0; h < height; h++ )
      {
        *s++ = '\t';
        for ( w = 0; w < width; w++ )
          {
            pixel = *icon++;

            a = (pixel & 0xff000000) >> 24;
            r = (pixel & 0x00ff0000) >> 16;
            g = (pixel & 0x0000ff00) >> 8;
            b = (pixel & 0x000000ff);

            brightness = (a / 255.0) * (1000 - ((299 * (r / 255.0)) +
                        (587 * (g / 255.0)) +
                        (114 * (b / 255.0))));

            if ( a_flags & FlagIsUTF8Locale )
              {
                static const char palette[][4] =
                  {
                    " ",
                    "\342\226\221",		/* 25% */
                    "\342\226\222",		/* 50% */
                    "\342\226\223",		/* 75% */
                    "\342\226\210",		/* 100% */
                  };

                idx = (brightness * (countof (palette) - 1)) / 1000;
                s += sprintf (s, "%s", palette[idx]);
              }
            else
              {
                static const char palette[] = " .'`,^:\";~-_+<>i!lI?/\\|()1{}[]rcvunxzjftLCJUYXZO0Qoahkbdpqwm*WMB8&%$#@";

                idx = (brightness * (countof (palette) - 2)) / 1000;
                *s++ = palette [idx];
              }
          }
        *s++ = '\n';
      }
    *s++ = '\n';
    *s = '\0';
}

#define ICON_MAX_WIDTH   144
#define ICON_MAX_HEIGHT  144

int
Print_Icons (const unsigned long *icon, int bytes)
{
    unsigned long width, height;
    int icon_size, arr_size, print_size;

    for ( arr_size = bytes / sizeof (unsigned long); arr_size != 0; arr_size -= icon_size )
    {
        /* dimension */
        width = *icon++;
        height = *icon++;

        icon_size = width * height;
        if ( icon_size > arr_size )
            break;

        /* type */
        verbose_c ('\t');
        Print ("Icon ", ColorKeyword);
        verbose_c ('(');

        /* width */
        Print_Unsigned (width, ColorValue);
        verbose_s (" x ");

        /* height */
        Print_Unsigned (height, ColorValue);
        verbose_s ("):\n");

        /* check dimension */
        if ( width > ICON_MAX_WIDTH || height > ICON_MAX_HEIGHT )
        {
            Print ("\t<not shown>", ColorUndefined);
            icon += icon_size;
            continue;
        }

        /* let's use xp_large_buffer so we don't waste the memory */
        print_size = ((width << 2) + 2) * height + 2;
        if ( print_size > xp_large_buffer.alloc_size &&
             Thunks_Double_Size (&xp_large_buffer, print_size) == NULL )
            return False;

        /* icon data */
        Print_Icon (icon, width, height);
        verbose_s ((char *) xp_large_buffer.items);
    }

    return True;
}

int
Print_Len_Text (const char *s, int len, Atom encoding)
{
    Remain_Buffer_Variables;
    Large_Buffer_Variables;
    XTextProperty textprop;
    char **list, **v;
    int n, count;
    wchar_t wc;
    
    /* Try to convert to local encoding. */
    textprop.encoding = encoding;
    textprop.format   = 8;
    textprop.value    = (unsigned char *) s;
    textprop.nitems   = len;

    if ( XmbTextPropertyToTextList (d_dpy, &textprop, &list, &count) != Success )
        return Print_Len_String (s, len, False);

    Large_Buffer_Init ();

    /* quote */
    if ( a_flags & FlagQuote )
        Large_Buffer_Put_Char ('\"');

    for ( v = list; ;v++ ) {
        s_remain_buffer = *v;
        n_remain_buffer = strlen (s_remain_buffer);

        /* max buffer size for the following loop is strlen (s) * 4
         * 6 ~ 4 chars for octet (\000), 2 for '"' and '\0'
         * and we need to check if remaining size (xp_large_buffer.nelements)
         * is greater or equal then the necessary size ($n) => $n > $r is
         * the same as $n - $r > 0 */
        n = (n_remain_buffer << 2) + 6 - xp_large_buffer.nelements;
        if ( n > 0 ) {
            /* the buffer is not enough and we need min. $n bytes plus the
             * current buffer size! */
            s = Thunks_String_Double_Size (&xp_large_buffer, n + xp_large_buffer.alloc_size);
            if ( s == NULL )
                return False;
        }
        
        while ( n_remain_buffer != 0 ) {
            n = mbtowc (&wc, s_remain_buffer, n_remain_buffer);

            if ( n != 0 && iswprint (wc) ) {
                /* $n chars */
                memcpy (s_large_buffer, s_remain_buffer, n);
                Large_Buffer_Offset (n);
                Remain_Buffer_Offset (n);
            } else {
                /* 4 chars */
                Large_Buffer_Put_Char ('\\');
                snprintf (s_large_buffer, 4, "%03o", (unsigned char) *s_remain_buffer);
                Large_Buffer_Offset (3);
                Remain_Buffer_Offset (1);
            }
        }

        if ( --count == 0 )
            break;

        /* 4 chars */
        Large_Buffer_Put_Char ('\\');
        Large_Buffer_Put_Char ('0');
        Large_Buffer_Put_Char ('0');
        Large_Buffer_Put_Char ('0');
    }

    /* free allocated list */
    XFreeStringList (list);

    /* unquote */
    if ( a_flags & FlagQuote )
        *s_large_buffer++ = '\"';

    *s_large_buffer = '\0';

    /* verbose */
    Print ((char *) xp_large_buffer.items, ColorValue);
    return True;
}

UTF8_Status
UTF8_Is_Valid (const char *s, unsigned int len)
{
    unsigned long codepoint = 0;
    int rem, i;
    unsigned char c;

    for (rem = 0, i = 0; i < len; i++) {
        c = (unsigned char) *s++;

        /* Order of type check:
         *   - Single byte code point
         *   - Non-starting byte of multi-byte sequence
         *   - Start of 2-byte sequence
         *   - Start of 3-byte sequence
         *   - Start of 4-byte sequence
         */
        if ( (c & 0x80) == 0 ) {
            if ( rem != 0 )
                return UTF8_ShortTail;

            rem = 0;
            codepoint = c;
        } else if ( (c & 0xC0) == 0x80 ) {
            if ( rem == 0 )
                return UTF8_LongTail;

            rem--;
            codepoint |= (c & 0x3F) << (rem * 6);
            if ( codepoint == 0 )
                return UTF8_Overlong;
        } else if ( (c & 0xE0) == 0xC0 ) {
            if ( rem != 0 )
                return UTF8_ShortTail;

            rem = 1;
            codepoint = ( c & 0x1F ) << 6;
            if ( codepoint == 0 )
                return UTF8_Overlong;
        } else if ( (c & 0xF0) == 0xE0 ) {
            if ( rem != 0 )
                return UTF8_ShortTail;

            rem = 2;
            codepoint = (c & 0x0F) << 12;
        } else if ( (c & 0xF8) == 0xF0 ) {
            if ( rem != 0 )
                return UTF8_ShortTail;

            rem = 3;
            codepoint = (c & 0x07) << 18;
            if ( codepoint > 0x10FFFF )
                return UTF8_ForbiddenValue;
        } else
            return UTF8_ForbiddenValue;
    }
    return UTF8_Valid;
}

void
UTF8_Status_Print_Error (const char *error)
{
    verbose_s ("<invalid UTF-8 string: ");
    Print (error, VerboseRed);
    verbose_c ('>');
}

int
Print_Len_Unicode (const char *s, int len)
{
    UTF8_Status validity;

    validity = UTF8_Is_Valid (s, len);
    if ( validity == UTF8_Valid )
        return Print_Len_String (s, len, a_flags & FlagIsUTF8Locale);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    switch (validity) {
        case UTF8_ForbiddenValue:
            UTF8_Status_Print_Error ("forbidden value");
            break;

        case UTF8_Overlong:
            UTF8_Status_Print_Error ("overlong encoding");
            break;

        case UTF8_ShortTail:
            UTF8_Status_Print_Error ("tail too short");
            break;

        case UTF8_LongTail:
            UTF8_Status_Print_Error ("tail too long");
            break;
    }
#pragma GCC diagnostic pop
    return Print_Len_String (s, len, False);
}

int
Print_Property_Rec (PropertyRec *p, VerboseColor c)
{
    /* name */
    Print (p->name, c);
    verbose_c (':');
    verbose_newline ();

    /* format */
    if ( (a_flags & FlagRaw) == 0 )
        verbose_s ("\tformat  = ");

    if ( !Print_String (p->format, 0) )
        return False;

    verbose_newline ();

    /* dformat format */
    if ( (a_flags & FlagRaw) == 0 )
        verbose_s ("\tdformat = ");

    if ( p->dformat != NULL ) {
        if ( !Print_String (p->dformat, 0) )
            return False;
    } else if ( (a_flags & FlagRaw) == 0 )
        Print ("<default>", ColorUndefined);
                
    verbose_newline ();
    return True;
}
                
int
Print_Property_Recs (const char *name, PropertyRec *p, unsigned int size, VerboseColor color)
{
    help (name);
        
    while ( size-- != 0 ) {
        if ( !Print_Property_Rec (p++, color) )
            return False;
    }

    verbose_newline ();
    return True;
}
