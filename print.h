/*
 * Created by Rene Durik (gadefoxren@gmail.com)  12.01.22
 */

#ifndef _PRINT_H_
#define _PRINT_H_

#include "verbose.h"
#include "misc.h"


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
void Print_Icon (const unsigned long *icon, unsigned int width, unsigned int height);
int Print_Icons (const unsigned long *icon, int bytes);
int Print_Len_Text (const char *s, int len, Atom encoding);
int Print_Len_Unicode (const char *s, int len);
int Print_Property_Rec (PropertyRec *p, VerboseColor c);
int Print_Property_Recs (const char *name, PropertyRec *p, unsigned int size, VerboseColor color);


#endif  /* _PRINT_H_ */
