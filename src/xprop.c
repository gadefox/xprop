/*
 * copyright 1990, 1998  the open group
 * copyright (c) 2000  the xfree86 project, inc.
 *
 * permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *   
 * the above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the software.
 * 
 * the software is provided "as is", without warranty of any kind, express
 * or implied, including but not limited to the warranties of
 * merchantability, fitness for a particular purpose and noninfringement.
 * in no event shall the open group be liable for any claim, damages or
 * other liability, whether in an action of contract, tort or otherwise,
 * arising from, out of or in connection with the software or the use or
 * other dealings in the software.
 *
 * except as contained in this notice, the name of the open group shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this software without prior written authorization
 * from the open group.
 */

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "xprop.h"
#include "verbose.h"
#include "misc.h"
#include "print.h"
#include "format.h"
#include "dsimple.h"
#include "thunk.h"
#include "showprop.h"
#include "args.h"


Window xp_target_win = None;
XFontStruct *xp_font;
unsigned long xp_font_prop;
Thunks xp_large_buffer;
unsigned int xp_term_width;


/*
 * Setup_Mapping: Routine to setup default atom to format, dformat mapping:
 */

#define ARC_DFORMAT         	"$@ ($#):\n"\
"$tarc at $0, $1\n"\
"$tsize: $2 by $3\n"\
"$tfrom angle $4 to angle $5\n"

#define RECTANGLE_DFORMAT	    "$@ (@#):\n"\
"$tupper left corner: $0, $1\n"\
"$tsize: $2 by $3\n"

#define RGB_COLOR_MAP_DFORMAT	"$@ (@#):\n"\
"$tcolormap id #: $0\n"\
"$tred-max: $1\n"\
"$tred-mult: $2\n"\
"$tgreen-max: $3\n"\
"$tgreen-mult: $4\n"\
"$tblue-max: $5\n"\
"$tblue-mult: $6\n"\
"$tbase-pixel: $7\n"\
"$tvisual id #: $8\n"\
"$tkill id #: $9\n"

#define WM_HINTS_DFORMAT	    "$@ ($#):\n"\
"?m0($tClient accepts input or input focus: $1\n)"\
"?m1($tInitial state is "\
"?$2=0(Don't Care State)"\
"?$2=1(Normal State)"\
"?$2=2(Zoomed State)"\
"?$2=3(Iconic State)"\
"?$2=4(Inactive State)"\
".\n)"\
"?m2($tbitmap id # to use for icon: $3\n)"\
"?m5($tbitmap id # of mask for icon: $7\n)"\
"?m3($twindow id # to use for icon: $4\n)"\
"?m4($tstarting position for icon: $5, $6\n)"\
"?m6($twindow id # of group leader: $8\n)"\
"?m8($tThe urgency hint bit is set\n)"

#define WM_ICON_SIZE_DFORMAT	"$@ ($#):\n"\
"$tminimum icon size: $0 by $1\n"\
"$tmaximum icon size: $2 by $3\n"\
"$tincremental size change: $4 by $5\n"

#define WM_SIZE_HINTS_DFORMAT   "$@ ($#):\n"\
"?m0($tuser specified location: $1, $2\n)"\
"?m2($tprogram specified location: $1, $2\n)"\
"?m1($tuser specified size: $3 by $4\n)"\
"?m3($tprogram specified size: $3 by $4\n)"\
"?m4($tprogram specified minimum size: $5 by $6\n)"\
"?m5($tprogram specified maximum size: $7 by $8\n)"\
"?m6($tprogram specified resize increment: $9 by $10\n)"\
"?m7($tprogram specified minimum aspect ratio: $11/$12\n"\
"  program specified maximum aspect ratio: $13/$14\n)"\
"?m8($tprogram specified base size: $15 by $16\n)"\
"?m9($twindow gravity: "\
"?$17=0(Forget)"\
"?$17=1(NorthWest)"\
"?$17=2(North)"\
"?$17=3(NorthEast)"\
"?$17=4(West)"\
"?$17=5(Center)"\
"?$17=6(East)"\
"?$17=7(SouthWest)"\
"?$17=8(South)"\
"?$17=9(SouthEast)"\
"?$17=10(Static)"\
"\n)"

#define WM_STATE_DFORMAT	    "$@ ($#):\n"\
"$twindow state: ?$0=0(Withdrawn)?$0=1(Normal)?$0=3(Iconic)\n"\
"$ticon window: $1\n"


static PropertyRec windowPropTable [] = {
    { "ARC",		             XA_ARC,		       "16iiccii",     ARC_DFORMAT },
    { "ATOM",	    	         XA_ATOM,	         "32a",	         0 },
    { "BITMAP",  	           XA_BITMAP,	       "32x",	         "$@ ($#): bitmap id # $0\n" },
    { "CARDINAL",	           XA_CARDINAL,	     "0c",	         0 },
    { "COLORMAP",        	   XA_COLORMAP,	     "32x",	         "$@ ($#): colormap id # $0\n" },
    { "CURSOR",		           XA_CURSOR,	       "32x",	         "$@ ($#): cursor id # $0\n" },
    { "DRAWABLE",	           XA_DRAWABLE, 	   "32x",          "$@ ($#): drawable id # $0\n" },
    { "FONT",	    	         XA_FONT,	         "32x",	         "$@ ($#): font id # $0\n" },
    { "INTEGER", 		         XA_INTEGER,	     "0i",	         0 },
    { "PIXMAP",		           XA_PIXMAP,	       "32x",	         "$@ ($#): pixmap id # $0\n" },
    { "POINT",		           XA_POINT,	       "16ii",         "$@ ($#) = $0, $1\n" },
    { "RECTANGLE",   	       XA_RECTANGLE, 	   "16iicc",       RECTANGLE_DFORMAT },
    { "RGB_COLOR_MAP",	     XA_RGB_COLOR_MAP, "32xcccccccxx", RGB_COLOR_MAP_DFORMAT},
    { "STRING",		           XA_STRING,	       "8s",	         0 },
    { "UTF8_STRING",		     0,	               "8u",	         0 },
    { "WINDOW",		           XA_WINDOW,	       "32x",	         "$@ ($#): window id # $0+\n" },
    { "VISUALID",    	       XA_VISUALID,	     "32x",	         "$@ ($#): visual id # $0\n" },
    { "WM_COLORMAP_WINDOWS", 0,	               "32x",          "$@ ($#): window id # $0+\n"},
    { "WM_COMMAND",	         XA_WM_COMMAND,	   "8s",	         "$@ ($#) = { $0+ }\n" },
    { "WM_HINTS",    	       XA_WM_HINTS,	     "32mbcxxiixx",  WM_HINTS_DFORMAT },
    { "WM_ICON_NAME",	       XA_WM_ICON_NAME,  "8t",	         0 },
    { "WM_ICON_SIZE",        XA_WM_ICON_SIZE,  "32cccccc",     WM_ICON_SIZE_DFORMAT},
    { "WM_NAME",	    	     XA_WM_NAME,	     "8t",	         0 },
    { "WM_PROTOCOLS",	       0,	               "32a",	         "$@ ($#): protocols  $0+\n"},
    { "WM_SIZE_HINTS",	     XA_WM_SIZE_HINTS, "32mii",        WM_SIZE_HINTS_DFORMAT },
    { "_NET_WM_ICON",        0,                "32o",          0 },
    { "WM_STATE",		         0,	               "32cx",         WM_STATE_DFORMAT }
};

#undef ARC_DFORMAT
#undef RECTANGLE_DFORMAT
#undef RGB_COLOR_MAP_DFORMAT
#undef WM_ICON_SIZE_DFORMAT
#undef WM_HINTS_DFORMAT
#undef WM_SIZE_HINTS_DFORMAT
#undef WM_STATE_DFORMAT

/* 
 * Font-specific mapping of property names to types:
 */
static PropertyRec fontPropTable [] = {

    /* XLFD name properties */
    { "FOUNDRY",		      0, 	 		        "32a",	0 },
    { "FAMILY_NAME",		  XA_FAMILY_NAME, "32a",	0 },
    { "WEIGHT_NAME",	    0,			        "32a",	0 },
    { "SLANT",			      0,			        "32a",	0 },
    { "SETWIDTH_NAME",	  0,			        "32a",	0 },
    { "ADD_STYLE_NAME",   0,			        "32a",	0 },
    { "PIXEL_SIZE",		    0,			        "32c",	0 },
    { "POINT_SIZE",		    XA_POINT_SIZE,  "32c",	0 },
    { "RESOLUTION_X",	    0,			        "32c",	0 },
    { "RESOLUTION_Y",		  0,			        "32c",	0 },
    { "SPACING",		      0,			        "32a",	0 },
    { "AVERAGE_WIDTH",    0,			        "32c",	0 },
    { "CHARSET_REGISTRY", 0,			        "32a",	0 },
    { "CHARSET_ENCODING", 0,			        "32a",	0 },

    /* other font properties referenced in the XLFD */
    { "QUAD_WIDTH",		       XA_QUAD_WIDTH,		       "32i",	0 },
    { "RESOLUTION",		       XA_RESOLUTION,		       "32c",	0 },
    { "MIN_SPACE",		       XA_MIN_SPACE,		       "32c",	0 },
    { "NORM_SPACE",		       XA_NORM_SPACE,		       "32c",	0 },
    { "MAX_SPACE",		       XA_MAX_SPACE,		       "32c",	0 },
    { "END_SPACE",		       XA_END_SPACE,		       "32c",	0 },
    { "SUPERSCRIPT_X",		   XA_SUPERSCRIPT_X,	     "32i",	0 },
    { "SUPERSCRIPT_Y",	 	   XA_SUPERSCRIPT_Y,	     "32i",	0 },
    { "SUBSCRIPT_X",		     XA_SUBSCRIPT_X,	 	     "32i",	0 },
    { "SUBSCRIPT_Y",		     XA_SUBSCRIPT_Y,		     "32i",	0 },
    { "UNDERLINE_POSITION",  XA_UNDERLINE_POSITION,	 "32i",	0 },
    { "UNDERLINE_THICKNESS", XA_UNDERLINE_THICKNESS, "32i",	0 },
    { "STRIKEOUT_ASCENT",    XA_STRIKEOUT_ASCENT,	   "32i",	0 },
    { "STRIKEOUT_DESCENT",   XA_STRIKEOUT_DESCENT,	 "32i",	0 },
    { "ITALIC_ANGLE",        XA_ITALIC_ANGLE,	       "32i",	0 },
    { "X_HEIGHT",		         XA_X_HEIGHT,		         "32i",	0 },
    { "WEIGHT",			         XA_WEIGHT,		           "32i",	0 },
    { "FACE_NAME",		       0,			                 "32a",	0 },
    { "COPYRIGHT",		       XA_COPYRIGHT,		       "32a",	0 },
    { "AVG_CAPITAL_WIDTH",	 0,			                 "32i",	0 },
    { "AVG_LOWERCASE_WIDTH", 0,			                 "32i",	0 },
    { "RELATIVE_SETWIDTH",	 0,			                 "32c",	0 },
    { "RELATIVE_WEIGHT",  	 0,			                 "32c",	0 },
    { "CAP_HEIGHT",		       XA_CAP_HEIGHT,		       "32c",	0 },
    { "SUPERSCRIPT_SIZE",	   0,			                 "32c",	0 },
    { "FIGURE_WIDTH",		     0,			                 "32i",	0 },
    { "SUBSCRIPT_SIZE",		   0,			                 "32c",	0 },
    { "SMALL_CAP_SIZE",		   0,			                 "32i",	0 },
    { "NOTICE",			         XA_NOTICE,		           "32a",	0 },
    { "DESTINATION",		     0,			                 "32c",	0 },

    /* other font properties */
    { "FONT",			 XA_FONT,		   "32a",	0 },
    { "FONT_NAME", XA_FONT_NAME, "32a",	0 }
};    


/*ARGSUSED*/
static int
ignore_errors (Display *dpy, XErrorEvent *ev)
{
    return 0;
}

const char *
Get_Atom_Name (Atom atom, PropertySource *src)
{
    const char *name;
    XErrorHandler handler;
    int namelen;

    /* window table */
    if ( a_flags & FlagXpropWindow ) {
        name = PropertyRec_Find_Atom_Name (windowPropTable, countof (windowPropTable), atom);
        if ( name != NULL ) {
            *src = PropertyWindowTable;
            return name;
        }
    } else {
        /* font table */
        name = PropertyRec_Find_Atom_Name (fontPropTable, countof (fontPropTable), atom);
        if ( name != NULL ) {
            *src = PropertyFontTable;
            return name;
        }
    }

    /* system */
    handler = XSetErrorHandler (ignore_errors);
    name = XGetAtomName (d_dpy, atom);
    XSetErrorHandler (handler);
    if ( name == NULL) {
        *src = PropertyUndefined;
        return Get_Undefined_Atom_Name (atom);
    }

    /* max length */
    namelen = strlen (name);
    if ( namelen >= MAX_STR )
        namelen = MAX_STR - 1;  /* 1 character for '\0' */

    /* thunk allocated size */
    if ( namelen >= xp_large_buffer.alloc_size &&  /* == due to '\0' character */
         Thunks_Double_Size (&xp_large_buffer, namelen + 1) == NULL )  /* double the buffer but we need at least niamelen + 1 characters */
        return NULL;

    memcpy (xp_large_buffer.items, name, namelen * sizeof (char));
    ((char *) xp_large_buffer.items) [namelen] = '\0';

    XFree ((char *) name);

    *src = PropertySystem;
    return (char *) xp_large_buffer.items;
}

static int
Show_Prop (RunParams *rp, Format *fmt, Atom atom, NameTypeFlags ntflags)
{
    const char *data;
    long length;
    Atom type;
    Thunks thunks_show_prop;
    int size, free_data;
    int ret = True;
    NameType ntdup, nt;
    Format curfmt;
    PropertySource psrc;

    /* init */
    ntdup.name = NULL;
    ntdup.type = NULL;

    /* we will release only items so no need to call Thunks_Init */
    thunks_show_prop.items = NULL;
    nt.flags = ntflags;

    /* format */
    if ( fmt != NULL )
        Format_Copy (&curfmt, fmt);
    else
        Format_Clear (&curfmt);

    /* property name */
    nt.name = Get_Atom_Name (atom, &psrc);
    if ( psrc == PropertyUndefined )
        nt.flags |= NameUndefined;

    if ( xp_target_win == -1 ) {
        /* font */
        data = Get_Font_Property_Data_And_Type (atom, &length, &type, &size);
        free_data = False;        
    }
    else {
        /* window */
        data = Get_Window_Property_Data_And_Type (atom, &length, &type, &size);
        free_data = True;
    }

    if ( data == NULL )
        return False;

    if ( size == 0 ) {
        error (msg_prop_not_found, nt.name);
        goto quit;  /* don't terminate the application */
    }

    Run_Params_Lookup_Formats (rp, atom, &curfmt);

    /* property type */
    if ( type != None ) {
        Run_Params_Lookup_Formats (rp, type, &curfmt);

        nt.type = Get_Atom_Name (type, &psrc);
        if ( psrc == PropertyUndefined )
            nt.flags |= TypeUndefined;
    } else
        nt.type = NULL;

    Format_Apply_Default (&curfmt);

    /* duplicate the name and the type */
    if ( !NameType_Dup (&ntdup, &nt) )
        goto quit;

    if ( curfmt.a.size != size && curfmt.a.size != 0 ) {
        error ("type mismatch: assumed size %d bits, actual size %d bits for property %s", curfmt.a.size, size, nt.name);
        goto quit;  /* don't terminate the application */
    }

    if ( !Thunk_Show_Prop_Break_Down_Property (&thunks_show_prop, data, (int)length, type, &curfmt.a, size) ||
         !Thunk_Show_Prop_Display_Property (&ntdup, &thunks_show_prop, &curfmt ) )
        ret = False; 

quit:

    free (thunks_show_prop.items);
    NameType_Free (&ntdup);

    if ( free_data )
        XFree ((char *) data);

    return ret;
}

static int
Show_All_Props (RunParams *rp)
{
    Atom *atoms, *acur;
    int count, i;
    XFontProp *fprop;

    if (xp_target_win != -1) {
        /* windows */
        atoms = XListProperties(d_dpy, xp_target_win, &count);
        for (i = count, acur = atoms; i != 0; i--, acur++) {
            if ( !Show_Prop (rp, NULL, *acur, PropertyWindow) ) {
                XFree(atoms);
                return False;
            }
        }
        XFree(atoms);
    } else {
        /* fonts */
        for (i = xp_font->n_properties, fprop = xp_font->properties; i != 0; i--, fprop++) {
            if ( !Show_Prop (rp, NULL, fprop->name, PropertyFont) )
                return False;
        }
    }
    return True;
}

static int 
Handle_Prop_Requests (RunParams *rparams)
{
    Thunks *sp;
    ThunkProperty *tp, *tp_new;
    Args cur;

    cur.count = rparams->args.count;
    if ( cur.count == 0 )
        /* if no prop referenced, by default list all properties for given window */
        return Show_All_Props (rparams);

    /* create thunk */
    sp = &rparams->spy_props;
    tp = (ThunkProperty *) Thunks_Create (sp, 0, sizeof (ThunkProperty));
    if ( tp == NULL )
        return False;

    /* parse non -- arguments */
    for ( cur.v = rparams->args.v; ; ) {
        if ( Args_Fetch_Next (&cur, NULL, False) == ArgsEnd )
            break;

        if ( !Args_Format_Parse_Name_Format (&tp->fmt, format_arg, cur.arg, FormatAtomOptional) )
            return False;

        /* get property name */
        tp->atom = Parse_Atom (cur.arg, True);
        if ( tp->atom == None ) {
            error (msg_prop_not_found, cur.arg);
            continue;
        }

        if ( !Show_Prop (rparams, &tp->fmt, tp->atom, PropertyWindow) )
            return False;
 
        /* increment $nelements and check allocated size */
        if ( ++sp->nelements != sp->alloc_size )
            tp++;
        else {
            tp_new = (ThunkProperty *) Thunks_Double_Size (sp, 0);
            if ( tp_new == NULL ) {
                /* don't forget to release memory */
                free ((char *) tp->fmt.d.delim);
                return False;
            }
            tp = tp_new + sp->nelements;
        }
    }

    return True;
}

/*
 * Routines for parsing command line:
 */

static void
Print_Help (void)
{
    static const char *help_message =
        "<options> include:\n"
        "  --help | -h                           print out a summary of command line options\n"
        "  --grammar | -g                        print out full grammar for command line\n"
        "  --print={font | window} |\n"
        "   -p {font | window}                   print out default formats\n"
        "  --dformat=<format>:<dformat>\n"
        "   -d <format>:<dformat>                set default format\n"
        "  --display=<host>:<dpy> |\n"
        "   -y <host>:<dpy>                      the X server to contact\n"
        "  --id=<id>                             resource id of window to examine\n"
        "  --name=<name> | -n <name>             name of window to examine\n"
        "  --font=<name>                         name of font to examine\n"
        "  --remove=<prop name>                  remove a property\n"
        "  --set=<prop name>:<value>             set a property to a given value\n"
        "  --root | -t                           examine the root window\n"
        "  --len=<n> | -l <n>                    dformat at most n bytes of any property\n"
        "  --raw | -r                            raw output\n"
        "  --formats=<filename> | -o <filename>  where to look for formats for properties\n"
        "  --frame | -m                          don't ignore window manager frames\n"
        "  --format=<name>:<format>[:dformat] |\n"
        "   -f <name> <format>[:dformat]         formats to use for property of given name\n"
        "  --spy | -s                            examine window properties forever\n"
        "  --version | -v                        print program version\n";

    help ("usage: [--<options>] [-ghmrstv] [atom[:format[:dformat]]]*");
    verbose_s (help_message);
    verbose_newline ();
}

static void
Print_Grammar (void)
{
    /* grammar */
    help ("grammar: [<dpy>] [<sel option>] <option>* <mapping>* <spec>*");
    verbose_string ("dpy        ", "--display=<host:dpy> | -y <host:dpy>");
    verbose_string ("sel option ", "{--root | -t} | --id=<id> | --font=<font> | {--name=<name> | -n <name>}");
    verbose_string ("option     ", "{--len=<n> | -l <n>} | {--raw | -r} | {--spy | -s} | {--formats=<file> | -o <file>} |");
    verbose_s ("            {--print={window | font} | -p {window | font}}");
    verbose_newline ();
    verbose_string ("mapping    ", "--remove=<propname> | {--format=<atom>:<fdpairo> | -f <atom> <fdpairo>} |");
    verbose_s ("            --set=<propname>:<value> | {--default=<fdpairm> | -d <fdpairm>}");
    verbose_newline ();
    verbose_string ("spec       ", "<atom> | <atom>:<fdpairo>");
    verbose_string ("fdpairo    ", "<format> | <fdpairm>");
    verbose_string ("fdpairm    ", "<format>:<dformat>");
    verbose_string ("format     ", "{0|8|16|32}{a|b|c|i|m|o|s|t|u|x}*");
    verbose_string ("dformat    ", "<unit><unit>*");
    verbose_string ("unit       ", "?<exp>(<exp>(*)) | <tab spaces> | $<n> | <list> | <prop name> | <prop type> | <delimiter> | <disp char>");
    verbose_string ("list       ", "$<n>+             ('$(n)<$delimiter>$(n+1)<$delimiter>$(n+2)..., $(nmax)')");
    verbose_string ("prop name  ", "$@");
    verbose_string ("prop type  ", "$#");
    verbose_string ("delimiter  ", "$^<sep>^          (default: ', ')");
    verbose_string ("tab spaces ", "$t");
    verbose_string ("exp        ", "<term> | <term>=<exp> | !<exp>");
    verbose_string ("term       ", "<n> | $<n> | m<n>");
    verbose_string ("disp char  ", "<norm char> | \\<non digit char> | \\<octal number> | $$");
    verbose_string ("norm char  ", "<any char except a digit, ?, $ or \\>");
    verbose_newline ();
}

/*
 * The Main Program:
 */

static int (*old_error_handler)(Display *dpy, XErrorEvent *ev);

static int spy_error_handler(Display *dpy, XErrorEvent *ev)
{
    if (ev->error_code == BadWindow || ev->error_code == BadMatch) {
        /* Window was destroyed: exit */
        verbose_c (0);
        a_flags &= ~FlagSpy;  /* error */
    }

    if (old_error_handler != NULL)
        return old_error_handler (dpy, ev);

    return 0;
}

static int
Spy (RunParams *rparams)
{
    XEvent event;
    ThunkProperty *tp;

    XSelectInput (d_dpy, xp_target_win, PropertyChangeMask | StructureNotifyMask);
    old_error_handler = XSetErrorHandler (spy_error_handler);

    for ( ;; ) {
        if ( (a_flags & FlagSpy) == 0 )
            break;

        fflush (stderr);

        XNextEvent (d_dpy, &event);
        if (event.type == DestroyNotify)
            break;

        if (event.type != PropertyNotify)
            continue;

        if ( rparams->spy_props.items == NULL ) {
            if ( !Show_Prop (rparams, NULL, event.xproperty.atom, PropertyWindow) )
                return False;

            continue;
        }

        tp = Thunks_Property_Find_Atom (&rparams->spy_props, event.xproperty.atom);
        if ( tp == NULL )
            continue;

        if ( !Show_Prop (rparams, &tp->fmt, event.xproperty.atom, PropertyWindow) )
            return False;
    }
    return True;
}

static int
Properties_Remove (RunParams *rparams)
{
    ThunkRemoveProp *t;
    int i;

    if ( rparams->remove_props.items == NULL )
        return True;

    if ( xp_target_win == -1 ) {
        error (msg_arg_only_windows, remove_arg);
        return False;
    }

    for (i = rparams->remove_props.nelements, t = (ThunkRemoveProp *) rparams->remove_props.items; i != 0; i--, t++)
        Remove_Property (xp_target_win, t->name);

    return True;
}

static int
Properties_Set (RunParams *rparams)
{
    ThunkSetProp *t;
    int i;

    if (rparams->set_props.items == NULL)
        return True;

    if ( xp_target_win == -1 ) {
        error (msg_arg_only_windows, set_arg);
        return False;
    }

    for (i = rparams->set_props.nelements, t = (ThunkSetProp *) rparams->set_props.items; i != 0; i--, t++) {
        if ( !Set_Property (rparams, xp_target_win, t->name, t->value) )
            return False;
    }

    return True;
}

static int
Print_Info (void)
{
    fflush (stdout);

    if ( a_flags & FlagHelp )
        Print_Help ();

    /* --print=grammar? */
    if ( a_flags & FlagGrammar )
        Print_Grammar ();

    if ( a_flags & (FlagDefaultWindow | FlagDefaultFont) ) {
        /* --print=window? */
        if ( a_flags & FlagDefaultWindow ) {
            if ( !Print_Property_Recs (window_arg, windowPropTable, countof (windowPropTable), ColorProperty) )
                return EXIT_FAILURE;
        }

        /* --print=font? */
        if ( a_flags & FlagDefaultFont ) {
            if ( !Print_Property_Recs (font_arg, fontPropTable, countof (fontPropTable), ColorFont) )
                return EXIT_FAILURE;
        }

        if ( !Format_Print_Default () )
            return EXIT_FAILURE;
    }

    /* --version? */
    if ( a_flags & FlagVersion ) {
        Print (VERSION, VerboseWhite);
        verbose_newline ();
    }

    return EXIT_SUCCESS;
}

static int 
Init_Target (void)
{
    /* init root */
    xp_target_win = RootWindow (d_dpy, d_screen);

    /* --id? */
    if ( a_winid != None )
        xp_target_win = a_winid;
    /* --name? */
    else if ( a_winname != NULL ) {
        xp_target_win = Window_With_Name (xp_target_win, a_winname);
        if ( xp_target_win == None ) {
            error ("no window with name '%s' exists", a_winname);
            return False;
        }
    /* ! --root? */
    } else if ( (a_flags & FlagRoot) == 0 ) {
        xp_target_win = Window_Select ( (a_flags & FlagFrameOnly) == 0 );
        if ( xp_target_win == -1 )
            return False;
    }

    return True;
}

static int
Run (RunParams *rp)
{
    char *env;

    /* open display */
    if ( !Display_Open (a_dispname) )
        return EXIT_FAILURE;

    /* parse all arguments */
    if ( !Run_Params_Parse_Args (rp) )
        return EXIT_FAILURE;

    if ( a_flags & (FlagHelp | FlagVersion | FlagGrammar | FlagDefaultWindow | FlagDefaultFont) ) {
        Print_Info ();
        return EXIT_SUCCESS;
    }

    /* Set up default atom to format, dformat mapping */
    if ( a_fontname == NULL ) {
        /* window properties */
        a_flags |= FlagXpropWindow;

        if ( !Thunks_Mapping_PropertyRec_Map (&rp->intern_props, windowPropTable, countof (windowPropTable)) )
            return EXIT_FAILURE;
    } else {
        /* fonts properties */
        xp_target_win = -1;

        xp_font = Font_Open (a_fontname);
        if ( xp_font == NULL )
             return EXIT_FAILURE;

        if ( !Thunks_Mapping_PropertyRec_Map (&rp->intern_props, fontPropTable, countof (fontPropTable)) )
             return EXIT_FAILURE;
    }

    /* env mappings */
    env = getenv ("XPROPFORMATS");
    if ( env != NULL && !Thunks_Mapping_Read_File (&rp->file_props, env) )
        return EXIT_FAILURE;

    /* select window */
    if ( xp_target_win == None && !Init_Target () )
        return EXIT_FAILURE;

    /* remove properties */
    if ( !Properties_Remove (rp ) )
        return EXIT_FAILURE;

    /* set properties */
    if ( !Properties_Set (rp ) )
        return EXIT_FAILURE;

    if ( rp->remove_props.items != NULL || rp->set_props.items != NULL )
        return EXIT_SUCCESS;

    if ( !Handle_Prop_Requests (rp) )
        return EXIT_FAILURE;

    if ( (a_flags & FlagSpy) && !Spy (rp) )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int
main (int argc, char **argv)
{
    RunParams rparams;
    int ret;

    /* args */
    prog_name = Get_Prog_Name (*argv);

    rparams.args.count = --argc;
    rparams.args.v     = ++argv;
    
    /* Set locale for XmbTextProptertyToTextList and iswprint(). */
    if ( !setlocale (LC_CTYPE, "") || !XSupportsLocale () )
        warn ("no locale support");
    
    if ( UTF8_Is_Locale () )
        a_flags |= FlagIsUTF8Locale;

    if ( is_truecolor_term () )
        a_flags |= FlagIsTrueColor;

    /* get term width */
    xp_term_width = get_term_width ();

    /* default format */
    Format_Set_Default (&fmt_default);

    /* run params */
    Run_Params_Init (&rparams);

    /* 4KB */
    if ( Thunks_Create (&xp_large_buffer, 1 << 12, sizeof (char)) == NULL )
        return EXIT_FAILURE;

    ret = Run (&rparams);

    /* we're done */
    Display_Close ();
    Run_Params_Free (&rparams);
    
    return ret;
}
