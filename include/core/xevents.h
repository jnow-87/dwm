#ifndef EVENTS_H
#define EVENTS_H


#include <X11/Xlib.h>


/* types */
typedef void (*cycle_callback_t)(void);


/* prototypes */
int xlib_events_init(void);
void xlib_cleanup(void);

int xlib_events_hdlr(void);
void xlib_event_handle(XEvent *ev);

int xerror_hdlr(Display *dpy, XErrorEvent *ee);
int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee);
int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee);

void key_cycle_start(cycle_callback_t complete);
void key_cycle_complete(void);
bool key_cycle_active(void);


#endif // EVENTS_H
