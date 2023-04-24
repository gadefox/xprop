/*
 * Copyright 2007 Kim woelders
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <X11/Xatom.h>

#include "clientwin.h"
#include "dsimple.h"


static Atom atom_wm_state = None;

/*
 * Check if window has given property
 */
static Bool
Window_Has_Property(Window win, Atom atom)
{
    Atom type_ret;
    int format_ret;
    unsigned char *prop_ret;
    unsigned long bytes_after, num_ret;

    type_ret = None;
    prop_ret = NULL;

    XGetWindowProperty(d_dpy, win, atom, 0, 0, False, AnyPropertyType,
                       &type_ret, &format_ret, &num_ret,
                       &bytes_after, &prop_ret);
    if (prop_ret)
        XFree (prop_ret);

    return type_ret != None;
}

/*
 * Check if window is viewable
 */
static Bool
Window_Is_Viewable(Window win)
{
    XWindowAttributes xwa;

    XGetWindowAttributes(d_dpy, win, &xwa);
    return xwa.class == InputOutput && xwa.map_state == IsViewable;
}

/*
 * Find a window that has WM_STATE set in the window tree below win.
 * Unmapped/unviewable windows are not considered valid matches.
 * Children are searched in top-down stacking order.
 * The first matching window is returned, None if no match is found.
 */
static Window
Find_Client_In_Children(Window win)
{
    Window root, parent, w;
    Window *children, *c;
    unsigned int n_children;
    int i;

    if (!XQueryTree(d_dpy, win, &root, &parent, &children, &n_children))
        return None;

    if (!children)
        return None;

    /* Check each child for WM_STATE and other validity */
    win = None;

    for ( i = n_children - 1, c = children + i; i >= 0; i--, c-- ) {  /* ~ c = &children [n_children - 1] */
        w = *c;
        if ( !Window_Is_Viewable (w) ) {
            *c = None;  /* Don't bother descending into this one */
            continue;
        }

        if ( !Window_Has_Property(w, atom_wm_state) )
            continue;

        /* Got one */
        win = w;
        goto done;
    }

    /* No children matched, now descend into each child */
    for ( i = n_children - 1, c = children + i; i >= 0; i--, c-- ) {
        w = *c;
        if (w == None)
            continue;

        win = Find_Client_In_Children (w);
        if (win != None)
            break;
    }
done:

    XFree (children);
    return win;
}

/*
 * Find virtual roots (_NET_VIRTUAL_ROOTS)
 */
static unsigned long *
Find_Roots(Window root, unsigned int *num)
{
    Atom type_ret;
    int format_ret;
    unsigned char *prop_ret;
    unsigned long bytes_after, num_ret;
    Atom atom;

    *num = 0;
    atom = XInternAtom(d_dpy, "_NET_VIRTUAL_ROOTS", False);
    if ( atom == None )
        return NULL;

    type_ret = None;
    prop_ret = NULL;

    if (XGetWindowProperty(d_dpy, root, atom, 0, 0x7fffffff, False,
                           XA_WINDOW, &type_ret, &format_ret, &num_ret,
                           &bytes_after, &prop_ret) != Success)
        return NULL;

    if (prop_ret && type_ret == XA_WINDOW && format_ret == 32) {
        *num = num_ret;
        return ((unsigned long *) prop_ret);
    }

    if (prop_ret)
        XFree(prop_ret);

    return NULL;
}

/*
 * Find child window at pointer location
 */
static Window
Find_Child_At_Pointer(Window win)
{
    Window root_return, child_return;
    int dummyi;
    unsigned int dummyu;

    XQueryPointer(d_dpy, win, &root_return, &child_return,
                  &dummyi, &dummyi, &dummyi, &dummyi, &dummyu);

    return child_return;
}

/*
 * Find client window at pointer location
 *
 * root   is the root window.
 * subwin is the subwindow reported by a ButtonPress event on root.
 *
 * If the WM uses virtual roots subwin may be a virtual root.
 * If so, we descend the window stack at the pointer location and assume the
 * child is the client or one of its WM frame windows.
 * This will of course work only if the virtual roots are children of the real
 * root.
 */
Window
Find_Client(Window root, Window subwin)
{
    unsigned long *roots, *r;
    unsigned int i, n_roots;
    Window win;

    /* Check if subwin is a virtual root */
    roots = Find_Roots(root, &n_roots);
    if ( roots != NULL ) {
        for (i = 0, r = roots; i < n_roots; i++, r++) {
            if (*r != subwin)
                continue;

            win = Find_Child_At_Pointer(subwin);
            if (win == None)
                return subwin;      /* No child - Return virtual root. */
        
            subwin = win;
            break;
        }

        XFree(roots);
    }

    if (atom_wm_state == None) {
        atom_wm_state = XInternAtom (d_dpy, "WM_STATE", False);
        if ( !atom_wm_state)
            return subwin;
    }

    /* Check if subwin has WM_STATE */
    if (Window_Has_Property(subwin, atom_wm_state))
        return subwin;

    /* Attempt to find a client window in subwin's children */
    win = Find_Client_In_Children(subwin);
    if (win != None)
        return win;             /* Found a client */

    /* Did not find a client */
    return subwin;
}
