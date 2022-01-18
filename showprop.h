/*
 * Created by Rene Durik (gadefoxren@gmail.com)  13.01.22
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
