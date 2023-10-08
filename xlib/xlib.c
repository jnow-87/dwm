#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <core/dwm.h>
#include <xlib/xlib.h>


/* local/static prototypes */
static int xerror_hdlr(Display *dpy, XErrorEvent *ee);


/* global functions */
int xlib_init(void){
	xlib_set_error_handler(xerror_hdlr);

	return 0;
}

void xlib_cleanup(void){
}

void xlib_sync(void){
	XSync(dwm.dpy, False);
}

int xlib_get_event(xevent_t *ev, bool blocking, long int mask){
	if(!blocking && XPending(dwm.dpy) == 0)
		return 0;

	if(mask != 0)
		return (XMaskEvent(dwm.dpy, mask, ev) != 0) ? -1 : 1;

	if(XNextEvent(dwm.dpy, ev))
		return -1;

	return 1;
}

void xlib_release_events(void){
	XAllowEvents(dwm.dpy, ReplayPointer, CurrentTime);
}

void xlib_set_error_handler(xlib_error_hdlr hdlr){
	if(hdlr == 0x0)
		hdlr = xerror_hdlr;

	XSetErrorHandler(hdlr);
}


/* local functions */
static int xerror_hdlr(Display *dpy, XErrorEvent *ee){
	// There's no way to check accesses to destroyed windows, thus those cases are
	// ignored (especially on UnmapNotify's). Other types of errors call Xlibs
	// default error handler, which may call exit.
	if(ee->error_code == BadWindow
	|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
	|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
	|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
	|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
	|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
	|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
	|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
		return 0;

	dwm_die("fatal: request code=%d, error code=%d\n", ee->request_code, ee->error_code);

	return -1;
}
