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

#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "dsimple.h"
#include "verbose.h"
#include "xprop.h"
#include "strutil.h"


Flags a_flags = FlagQuote;
int a_max_len = MAX_STR;
const char *a_dispname = NULL;
const char *a_fontname = NULL;
const char *a_winname = NULL;
Window a_winid = None;

const char spy_arg[]     = "spy";
const char len_arg[]     = "len";
const char remove_arg[]  = "remove";
const char format_arg[]  = "format";
const char frame_arg[]   = "frame";
const char formats_arg[] = "formats";
const char root_arg[]    = "root";
const char name_arg[]    = "name";
const char id_arg[]      = "id";
const char display_arg[] = "display";
const char font_arg[]    = "font";
const char window_arg[]  = "window";
const char raw_arg[]     = "raw";
const char help_arg[]    = "help";
const char version_arg[] = "version";
const char grammar_arg[] = "grammar";
const char notype_arg[]  = "notype";
const char print_arg[]   = "print";
const char set_arg[]     = "set";
const char dformat_arg[] = "dformat";


/*
 * Args
 */

void
Args_Copy_Remaining (Args *dst, Args *src)
{
    Args copy, cur;

    cur.count = src->count;
    cur.v     = src->v;

    copy.count = dst->count;
    copy.v     = dst->v;

    /* copy counting arguments */
    *copy.v++ = src->arg;
    copy.count++;

    while ( cur.count != 0 ) {
        *copy.v++ = *cur.v++;
        cur.count--;
        copy.count++;
    }

    dst->count = copy.count;
}

ArgsStatus
Args_Fetch_Next (Args *args, const char *argname, int no_parse )
{
    char *name;
    char *value;
    ArgsStatus stat;

    /* counting arguments? */
    if ( args->count == 0 ) {
        if ( argname != NULL )
            error ( msg_arg_missing, argname);
        return ArgsEnd;
    }

    /* get next */
    args->count--;

    name = *args->v++;
    args->arg = name;

    if ( no_parse )
        return ArgsWord;

    /* check '--' and '-' */
    if ( *name == '-' ) {
        if ( *++name == '-' ) {
            /* parse name/value pair */
            value = strchr (++name, '=');
            if ( value == NULL )
                args->value = NULL;
            else {
                *value++ = '\0';
                args->value = value;
            }
            stat = ArgsDouble;
        } else
            stat = ArgsSingle;
    } else
        stat = ArgsWord;

    args->name = name;
    return stat;
}

int
Args_Format_Parse_Default (const char *arg_name, char *arg_value)
{
    if ( !Args_Format_Parse (&fmt_default, arg_name, arg_value, FormatMandatory) )
        return False;

     /* delim */
     if ( fmt_default.d.delim == NULL )
         fmt_default.d.delim = def_delim;

     return True;
}

int
Args_Format_Parse (Format *fmt, const char *arg_name, char *arg_value, FormatParseType type)
{
    char *dformat;

    /* possibilities:
     *   format:dformat
     *   format
     */

    /* dformat */
    dformat = strchr (arg_value, ':');
    if ( dformat == NULL ) {
        /* no dformat available */
        if ( type == FormatDisplayOptional )
            Format_Display_Clear (&fmt->d);
        else {
            error (msg_missing, arg_name, dformat_arg);
            return False;
        }
    } else {
        /* dformat is available so it doesn't matter if it's optional */
        *dformat++ = '\0';
        if ( *dformat == '\0' ) {
            error (msg_missing, arg_name, dformat_arg);
            return False;
        }

        if ( !Format_Display_Parse (&fmt->d, dformat) )
            return False;
    }

    /* format */
    if ( *arg_value == '\0' ) {
        error (msg_missing, arg_name, format_arg);
        return False;
    }

    if ( !Format_Atom_Parse (&fmt->a, arg_value) ) {
        /* $delim is duplicated */
        free ((char *) fmt->d.delim);
        return False;
    }

    return True;
}

int
Args_Format_Parse_Name_Format (Format *fmt, const char *arg_name, char *arg_value, FormatParseType type)
{
    /* --format==name:format[:dformat] */

    /* parse name */
    arg_value = strchr (arg_value, ':');
    if ( arg_value == NULL ) {
        /* only name? */
        if ( type == FormatAtomOptional ) {
            Format_Clear (fmt);
            return True;
        }

        error (msg_arg_missing, arg_name);
        return False;
    }
    *arg_value++ = '\0';

    /* parse format:dformat pair */
    return Args_Format_Parse (fmt, arg_name, arg_value, type);
}


/*
 * Run params
 */

int
Run_Params_Parse_Args (RunParams *rparams)
{
    Args cur, copy;
    ArgsStatus stat;
    char c;
    Window w;

    /* init */
    cur.count = rparams->args.count;
    cur.v     = rparams->args.v;
    
    copy.count = 0;
    copy.v     = cur.v;
   
    /* handle arguments */
    for ( ;; ) {
        stat = Args_Fetch_Next (&cur, NULL, False);
        if ( stat == ArgsEnd )
            break;
        
        if ( stat == ArgsDouble ) {
            /* -- */
            if ( *cur.name == '\0' && cur.value == NULL ) {
                /* copy remaining arguments */
                Args_Copy_Remaining (&copy, &cur);
                break;
            }
            /* --root */
            if ( strcmp (cur.name, root_arg) == 0 ) {
                a_flags |= FlagRoot;
                continue;
            }
            /* --display */
            if ( strcmp (cur.name, display_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                a_dispname = cur.value;
                continue;
            }
            /* --name */
            if ( strcmp (cur.name, name_arg) == 0) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                a_winname = cur.value;
                continue;
            }
            /* --id */
            if ( strcmp (cur.name, id_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                
                sscanf (cur.value, "0x%lx", &w);
                if ( w == None )
                    sscanf (cur.value, "%lu", &w);
                if ( w == None ) {
                    error (msg_invalid_winid, cur.value);
                    return False;
                }
                a_winid = w;
                continue;
            }
            /* --font */
            if ( strcmp (cur.name, font_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                a_fontname = cur.value;
                continue;
            }
            /* --raw */
            if ( strcmp (cur.name, raw_arg) == 0 ) {
                a_flags |= FlagRaw;
                a_flags &= ~FlagQuote;
                continue;
            }
            /* --help */
            if ( strcmp (cur.name, help_arg) == 0 ) {
                a_flags |= FlagHelp;
                continue;
            }
            /* --version */
            if ( strcmp (cur.name, version_arg) == 0 ) {
                a_flags |= FlagVersion;
                continue;
            }
            /* --grammar */
            if ( strcmp (cur.name, grammar_arg) == 0 ) {
                a_flags |= FlagGrammar;
                continue;
            }
            /* --print */
            if ( strcmp (cur.name, print_arg) == 0 ) {
                if ( cur.value == NULL )
                    a_flags |= FlagDefaultWindow | FlagDefaultFont;
                else if ( strcmp (cur.value, window_arg) == 0 )
                    a_flags |= FlagDefaultWindow;
                else if ( strcmp (cur.value, font_arg) == 0 )
                    a_flags |= FlagDefaultFont;
                else {
                    error (msg_arg_unknown_param, cur.arg, cur.value);
                    return False;
                }
                continue;
            }
            /* --dformat */
            if ( strcmp (cur.name, dformat_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                /* both format and dformat are mandatory */
                if ( !Args_Format_Parse_Default (cur.arg, cur.value) )
                    return False;
                continue;
            }
            /* --notype */
            if ( strcmp (cur.name, notype_arg) == 0 ) {
                warn ("%s is depreciated, use format without '$#' tag instead", cur.arg);
                continue;
            }
            /* --spy */
            if ( strcmp (cur.name, spy_arg) == 0) {
                a_flags |= FlagSpy;
                continue;
            }
            /* --len */
            if ( strcmp (cur.name, len_arg) == 0) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                /* parse # */
                s_parse_uint (cur.value, &a_max_len);
                if ( a_max_len == -1 ) {
                    error (msg_invalid_number, len_arg, cur.value);
                    return False;
                }
                continue;
            }
            /* --formats */
            if ( strcmp (cur.name, formats_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                if ( !Thunks_Mapping_Read_File (&rparams->file_props, cur.value) )
                    return False;
                continue;
            }
            /* --remove */
            if ( strcmp (cur.name, remove_arg) == 0) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                if ( Thunks_Remove_Prop_Init_And_Add (&rparams->remove_props, cur.value) == NULL )
                    return False;
                continue;
            }
            /* --set */
            if ( strcmp (cur.name, set_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                if ( Thunks_Set_Prop_Init_And_Add (&rparams->set_props, cur.value) == NULL )
                    return False;
                continue;
            }
            /* --frame */
            if ( strcmp (cur.name, frame_arg) == 0) {
                a_flags |= FlagFrameOnly;
                continue;
            }
            /* --format */
            if ( strcmp (cur.name, format_arg) == 0 ) {
                if ( cur.value == NULL ) {
                    error (msg_arg_missing, cur.arg);
                    return False;
                }
                if ( !Thunks_Mapping_Format_Parse (&rparams->mappings, format_arg, cur.value) )
                    return False;
                continue;
            }
            /* unknown argument */
            error (msg_arg_unknown, cur.arg);
            return False;
        }
        
        if (stat == ArgsSingle ) {
            if ( *cur.name == '\0' ) {
                /* copy remaining arguments */
                Args_Copy_Remaining (&copy, &cur);
                break;
            }
            /* -d ~ --dformat */
            if ( strcmp (cur.name, "d") == 0 ) {
                if ( Args_Fetch_Next (&cur, dformat_arg, True) == ArgsEnd )
                    return False;
                /* both format and dformat are mandatory */
                if ( !Args_Format_Parse_Default (dformat_arg, cur.arg) )
                    return False;
                
                continue;
            }
            /* -l ~ --len */
            if ( strcmp (cur.name, "l") == 0 ) {
                if ( Args_Fetch_Next (&cur, len_arg, True) == ArgsEnd )
                    return False;
                /* parse # */
                s_parse_uint (cur.arg, &a_max_len);
                if ( a_max_len == -1 ) {
                    error (msg_invalid_number, len_arg, cur.value);
                    return False;
                }
                continue;
            }
            /* -y ~ --display */
            if ( strcmp (cur.name, "y") == 0 ) {
                if ( Args_Fetch_Next (&cur, display_arg, True) == ArgsEnd )
                    return False;
                a_dispname = cur.arg;
                continue;
            }
            /* -f ~ --format */
            if ( strcmp (cur.name, "f") == 0 ) {
                if ( !Thunks_Mapping_Format_Parse_Args (&rparams->mappings, &cur) )
                    return False;
                continue;
            }
            /* -o ~ --formats */
            if ( strcmp (cur.name, "o") == 0 ) {
                if ( Args_Fetch_Next (&cur, formats_arg, True) == ArgsEnd )
                    return False;

                if ( !Thunks_Mapping_Read_File (&rparams->file_props, cur.arg) )
                    return False;
                continue;
            }
            /* -n ~ --name */
            if ( strcmp (cur.name, "n") == 0 ) {
                if ( Args_Fetch_Next (&cur, name_arg, True) == ArgsEnd )
                    return False;
                a_winname = cur.arg;
                continue;
            }
            /* -p ~ --print */
            if ( strcmp (cur.name, "p") == 0 ) {
                if ( Args_Fetch_Next (&cur, print_arg, True) == ArgsEnd )
                    return False;

                if ( strcmp (cur.arg, window_arg) == 0 )
                    a_flags |= FlagDefaultWindow;
                else if ( strcmp (cur.arg, font_arg) == 0 )
                    a_flags |= FlagDefaultFont;
                else {
                    error (msg_arg_unknown_param, print_arg, cur.arg);
                    return False;
                }
                continue;
            }
            
            for ( c = *cur.name; c != '\0'; c = *++cur.name ) {
                /* -s ~ --spy */
                if ( c == 's' )
                    a_flags |= FlagSpy;
                /* -e ~ --frame */
                else if ( c == 'm' )
                    a_flags |= FlagFrameOnly;
                /* -r ~ --root */
                else if ( c == 't' )
                    a_flags |= FlagRoot;
                /* -g ~ --grammar */
                else if ( c == 'g' )
                    a_flags |= FlagGrammar;
                /* -w ~ --raw */
                else if ( c == 'r' ) {
                    a_flags |= FlagRaw;
                    a_flags &= ~FlagQuote;
                /* -h ~ --help */
                } else if ( c == 'h' )
                    a_flags |= FlagHelp;
                /* -v ~- --version */
                else if ( c == 'v' )
                    a_flags |= FlagVersion;
                /* unknown argument */
                else {
                    error (msg_arg_unknown_char, c);
                    return False;
                }
            }
        } else {
            /* copy argument */
            *copy.v++ = cur.arg;
            copy.count++;
        }
    }
    rparams->args.count = copy.count;
    return True;
}

void
Run_Params_Lookup_Formats (RunParams *rp, Atom atom, Format *fmt)
{
    /* loop up in reverse order */
    Thunks_Mapping_Lookup_Formats (&rp->mappings, atom, fmt);
    Thunks_Mapping_Lookup_Formats (&rp->file_props, atom, fmt);
    Thunks_Mapping_Lookup_Formats (&rp->intern_props, atom, fmt);
}

void
Run_Params_Free (RunParams *rparams)
{
    /* strings are not duplicated because the values are parsed from the arguments */
    Thunks_Free (&rparams->remove_props);
    Thunks_Free (&rparams->set_props);
    Thunks_Free (&xp_large_buffer);

    /* we duplicate format strings */
    Thunks_Property_Free (&rparams->spy_props);
    
    Thunks_Mapping_Free (&rparams->mappings, False);  /* free only $delim string */
    Thunks_Mapping_Free (&rparams->file_props, True);  /* free whole structure */
    Thunks_Mapping_Free (&rparams->intern_props, False);  /* free only $delim string */
}

void
Run_Params_Init (RunParams *rparams)
{
    /* thunks */
    Thunks_Init (&rparams->spy_props);
    Thunks_Init (&rparams->mappings);
    Thunks_Init (&rparams->file_props);
    Thunks_Init (&rparams->intern_props);
    Thunks_Init (&rparams->remove_props);
    Thunks_Init (&rparams->set_props);
}

/*
 * Thunks
 */

int
Thunks_Mapping_Format_Parse_Args (Thunks *thunks, Args *args)
{
    char *name;
    Format fmt;
    Atom atom;
    
    /* -f name <format>[:<dformat>] */
    
    /* name */
    if ( Args_Fetch_Next (args, format_arg, True) == ArgsEnd )
        return False;
    
    name = args->arg;
    
    /* format (mandatory) and dformat (optional) */
    if ( Args_Fetch_Next (args, format_arg, True) == ArgsEnd )
        return False;
    
    if ( !Args_Format_Parse (&fmt, format_arg, args->arg, True) )
        return False;
    
    /* atom */
    atom = Parse_Atom (name, False);
    
    /* add new mapping */
    if ( !Thunks_Mapping_Init_And_Add (thunks, atom, &fmt) ) {
        /* only $delim is duplicated */
        free ((char *) fmt.d.delim);
        return False;
    }
    
    return True;
}
