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

#include <X11/cursorfont.h>

#include <string.h>

#include "dsimple.h"
#include "clientwin.h"
#include "verbose.h"


/*
 * Just_display: A group of routines designed to make the writting of simple
 *               X11 applications which open a display but do not open
 *               any windows much faster and easier.  Unless a routine says
 *               otherwise, it may be assumed to require d_dpy,
 *               and d_screen already defined on entry.
 *
 * Written by Mark Lillibridge.   Last updated 7/1/87
 */


/* This stuff is defined in the calling program by just_display.h */
Display *d_dpy = NULL;
int d_screen = 0;


/*
 * Display_Display: Routine to open a display with correct error handling.
 *               Does not require d_dpy or d_screen defined on entry.
 */
int Display_Open (const char *name)
{
	d_dpy = XOpenDisplay (name);
	if ( d_dpy == NULL ) {
	    error ("unable to open display: %s", XDisplayName (name));
	    return False;
	}

    d_screen = XDefaultScreen (d_dpy);
	return True;
}

/*
 * Display_Close: Close display
 */
void Display_Close (void)
{
    if (d_dpy == NULL)
        return;
      
    XCloseDisplay (d_dpy);
    d_dpy = NULL;
}


/*
 * Open_Font: This routine opens a font with error handling.
 */
XFontStruct *Font_Open (const char *name)
{
	XFontStruct *font;

    font = XLoadQueryFont (d_dpy, name);
	if (!font) {
        error ("unable to open font: %s", name);
        return NULL;
    }

	return font;
}


/*
 * Routine to let user select a window using the mouse
 */

Window Window_Select (int descend)
{
    int status;
    Cursor cursor;
    XEvent event;
    Window target_win = None;
    Window root;
    int buttons = 0;

    help ("select the window about which you would like information by clicking the mouse in that window.");

    /* Make the target cursor */
    root = RootWindow (d_dpy, d_screen);
    cursor = XCreateFontCursor (d_dpy, XC_crosshair);

    /* Grab the pointer using target cursor, letting it room all over */
    status = XGrabPointer (d_dpy, root, False,
			ButtonPressMask | ButtonReleaseMask, GrabModeSync,
			GrabModeAsync, root, cursor, CurrentTime);

    if (status != GrabSuccess) {
        error ("can't grab the mouse.");
        return -1;
    }

    /* Let the user select a window... */
    while (target_win == None || buttons != 0) {
        /* allow one more event */
        XAllowEvents(d_dpy, SyncPointer, CurrentTime);
        XWindowEvent(d_dpy, root, ButtonPressMask | ButtonReleaseMask, &event);
        
        switch (event.type) {
            case ButtonPress:
                if (target_win == None) {
	                target_win = event.xbutton.subwindow; /* window selected */
	                if (target_win == None)
                        target_win = root;
                }
                buttons++;
                break;

            case ButtonRelease:
                if (buttons != 0) /* there may have been some down before we started */
                    buttons--;
                break;
        }
    } 

    XUngrabPointer (d_dpy, CurrentTime);      /* Done with pointer */

    if (!descend || target_win == root)
        return target_win;

    target_win = Find_Client (root, target_win);
    return target_win;
}


/*
 * Window_With_Name: routine to locate a window with a given name on a display.
 *                   If no window with the given name is found, 0 is returned.
 *                   If more than one window has the given name, the first
 *                   one found will be returned.  Only top and its subwindows
 *                   are looked at.  Normally, top should be the RootWindow.
 */
Window Window_With_Name(
    Window top,
    const char *name)
{
	Window *children, *p;
    Window dummy;
	unsigned int nchildren;
	Window w;
	char *window_name;

	if ( XFetchName(d_dpy, top, &window_name) && !strcmp(window_name, name) )
	    return top;

	if ( !XQueryTree(d_dpy, top, &dummy, &dummy, &children, &nchildren) )
	    return None;

	for ( p = children; nchildren != 0; nchildren--, p++ ) {
		w = Window_With_Name (*p, name);
		if ( w != None) {
            XFree ((char *)children);
            return w;
    	}
    }

	if (children != NULL)
        XFree ((char *)children);
    
	return None;
}
