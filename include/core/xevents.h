#ifndef EVENTS_H
#define EVENTS_H


#include <xlib/xlib.h>


/* prototypes */
int xevents_handle_events(void);
void xevents_handle_event(xevent_t *ev);


#endif // EVENTS_H
