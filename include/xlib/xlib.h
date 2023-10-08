#ifndef XLIB_H
#define XLIB_H


#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xlib.h>


/* types */
typedef XEvent xevent_t;
typedef int (*xlib_error_hdlr)(Display *dpy, XErrorEvent *ee);


/* prototypes */
int xlib_init(void);
void xlib_cleanup(void);

void xlib_sync(void);

int xlib_get_event(xevent_t *ev, bool blocking, long int mask);
void xlib_release_events(void);

void xlib_set_error_handler(xlib_error_hdlr hdlr);


#endif // XLIB_H
