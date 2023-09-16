#ifndef XLIB_EVENTS_H
#define XLIB_EVENTS_H


#include <X11/Xlib.h>


/* prototypes */
void handle_event(XEvent *ev);

int xerror_hdlr(Display *dpy, XErrorEvent *ee);
int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee);
int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee);


#endif // XLIB_EVENTS_H
