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

#ifndef _ARGS_H_
#define _ARGS_H_

#include "thunk.h"


typedef enum {
    FlagIsUTF8Locale  = (1 << 0),
    FlagIsTrueColor   = (1 << 1),
    FlagVersion       = (1 << 2),
    FlagHelp          = (1 << 3),
    FlagGrammar       = (1 << 4),
    FlagDefaultWindow = (1 << 5),
    FlagDefaultFont   = (1 << 6),
    FlagRaw           = (1 << 7),
    FlagQuote         = (1 << 8),
    FlagFrameOnly     = (1 << 9),
    FlagRoot          = (1 << 10),
    FlagSpy           = (1 << 11),
    FlagXpropWindow   = (1 << 12),
    FlagXpropFont     = 0 
} Flags;


extern Flags a_flags;
extern int a_max_len;
extern const char *a_dispname;
extern const char *a_fontname;
extern const char *a_winname;
extern Window a_winid;

extern const char spy_arg[];
extern const char len_arg[];
extern const char remove_arg[];
extern const char format_arg[];
extern const char frame_arg[];
extern const char formats_arg[];
extern const char root_arg[];
extern const char name_arg[];
extern const char id_arg[];
extern const char display_arg[];
extern const char font_arg[];
extern const char window_arg[];
extern const char raw_arg[];
extern const char help_arg[];
extern const char version_arg[];
extern const char grammar_arg[];
extern const char notype_arg[];
extern const char print_arg[];
extern const char set_arg[];
extern const char dformat_arg[];


/*
 * Args
 */

typedef enum {
    FormatMandatory,
    FormatAtomOptional,
    FormatDisplayOptional
} FormatParseType;

typedef struct {
    int count;
    char **v;
    char *arg;
    char *name;
    char *value;
} Args;

typedef enum {
    ArgsEnd,
    ArgsDouble,
    ArgsSingle,
    ArgsWord
} ArgsStatus;

void Args_Copy_Remaining (Args *copy, Args *args);
ArgsStatus Args_Fetch_Next (Args *args, const char *argname, int no_parse );

int Args_Format_Parse (Format *fmt, const char *arg_name, char *arg_value, FormatParseType type);
int Args_Format_Parse_Name_Format (Format *fmt, const char *arg_name, char *arg_value, FormatParseType type);
int Args_Format_Parse_Default (const char *arg_name, char *arg_value);


/*
 * Thunks
 */

int Thunks_Mapping_Format_Parse_Args (Thunks *thunks, Args *args);


/*
 * Run params
 */

typedef struct {
    Thunks spy_props;     /* ThunkProperty array */
    Thunks remove_props;  /* ThunkRemoveProp array */
    Thunks set_props;     /* ThunkSetProp array */
    Thunks mappings;      /* ThunkMapping array */
    Thunks file_props;    /* ThunkMapping array */
    Thunks intern_props;  /* ThunkMapping array */
    Args args;
} RunParams;


int Run_Params_Parse_Args (RunParams *rparams);
void Run_Params_Lookup_Formats (RunParams *rp, Atom atom, Format *fmt);
void Run_Params_Free (RunParams *rparams);
void Run_Params_Init (RunParams *rparams);


#endif  /* _ARGS_H_ */
