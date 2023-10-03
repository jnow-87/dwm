#ifndef XLIB_EVENTS_H
#define XLIB_EVENTS_H


#include <X11/Xlib.h>


/* prototypes */
int xlib_events_init(void);
void xlib_cleanup(void);

int xlib_events_hdlr(void);
void xlib_event_handle(XEvent *ev);

int xerror_hdlr(Display *dpy, XErrorEvent *ee);
int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee);
int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee);


#endif // XLIB_EVENTS_H
