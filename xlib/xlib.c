#include <stdbool.h>
#include <stdlib.h>
#include <locale.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <core/dwm.h>
#include <xlib/gfx.h>
#include <xlib/input.h>
#include <xlib/xlib.h>
#include <utils/log.h>


/* local/static prototypes */
static int xerror_hdlr(Display *dpy, XErrorEvent *ee);
static int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee);
static void check_other_wm_running(void);


/* global functions */
int xlib_init(void){
	XSetWindowAttributes wa;


	xlib_set_error_handler(xerror_hdlr);

	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		INFO("warning: no locale support\n");

	dwm.dpy = XOpenDisplay(0x0);

	if(dwm.dpy == 0)
		return ERROR("opening display\n");

	check_other_wm_running();

	dwm.screen = DefaultScreen(dwm.dpy);
	dwm.screen_width = DisplayWidth(dwm.dpy, dwm.screen);
	dwm.screen_height = DisplayHeight(dwm.dpy, dwm.screen);
	dwm.root = RootWindow(dwm.dpy, dwm.screen);

	dwm.gfx = gfx_create(dwm.screen_width, dwm.screen_height);

	if(dwm.gfx == 0x0)
		return STRERROR("creating grafix context");

	dwm.numlock_mask = input_get_numlock_mask();

	/* init atoms */
	wmatom_init(WM_PROTOCOLS, "WM_PROTOCOLS");
	wmatom_init(WM_DELETE_WINDOW, "WM_DELETE_WINDOW");
	wmatom_init(WM_TAKEFOCUS, "WM_TAKE_FOCUS");
	wmatom_init(WM_STATE, "WM_STATE");

	netatom_init(NET_ACTIVE_WINDOW, "_NET_ACTIVE_WINDOW", XA_WINDOW, 32);
	netatom_init(NET_SUPPORTED, "_NET_SUPPORTED", XA_ATOM, 32);
	netatom_init(NET_WM_NAME, "_NET_WM_NAME", XInternAtom(dwm.dpy, "UTF8_STRING", False), 8);
	netatom_init(NET_WM_CHECK, "_NET_SUPPORTING_WM_CHECK", XA_WINDOW, 32);
	netatom_init(NET_WM_STATE, "_NET_WM_STATE", XA_ATOM, 32);
	netatom_init(NET_WM_FULLSCREEN, "_NET_WM_STATE_FULLSCREEN", None, -1);
	netatom_init(NET_WM_VERTMAX, "_NET_WM_STATE_MAXIMIZED_VERT", None, -1);
	netatom_init(NET_WM_HORMAX, "_NET_WM_STATE_MAXIMIZED_HORZ", None, -1);
	netatom_init(NET_CLIENT_LIST, "_NET_CLIENT_LIST", XA_WINDOW, 32);

	/* supporting window for NET_WM_CHECK */
	// this is a requirement to indicate a conforming window manager, cf.
	// https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html#idm45771211439200
	dwm.wmcheck = XCreateSimpleWindow(dwm.dpy, dwm.root, 0, 0, 1, 1, 0, 0, 0);
	netatom_set(NET_WM_CHECK, dwm.wmcheck, (unsigned char *)&dwm.wmcheck, 1);
	netatom_set(NET_WM_NAME, dwm.wmcheck, (unsigned char *)"dwm", 3);
	netatom_set(NET_WM_CHECK, dwm.root, (unsigned char *)&dwm.wmcheck, 1);

	/* extended window manager hints (EWMH) support per view */
	netatom_delete(NET_SUPPORTED, dwm.root);
	netatom_set(NET_CLIENT_LIST, dwm.root, 0x0, 0);

	for(size_t i=0; i<NNETATOMS; i++)
		netatom_append(NET_SUPPORTED, dwm.root, (unsigned char*)&dwm.netatoms[i].property);

	/* select events */
	wa.cursor = dwm.gfx->cursors[CUR_NORM];

	wa.event_mask = SubstructureRedirectMask
				  | SubstructureNotifyMask
				  | StructureNotifyMask
				  | PropertyChangeMask
				  | KeyPressMask
				  | ButtonPressMask
				  | FocusChangeMask
				  | ExposureMask
				  ;

	XChangeWindowAttributes(dwm.dpy, dwm.root, CWEventMask | CWCursor, &wa);
	XSelectInput(dwm.dpy, dwm.root, wa.event_mask);

	return 0;
}

void xlib_cleanup(void){
	XUngrabKey(dwm.dpy, AnyKey, AnyModifier, dwm.root);
	XDestroyWindow(dwm.dpy, dwm.wmcheck);

	gfx_free(dwm.gfx);
	xlib_sync();

	XSetInputFocus(dwm.dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	netatom_delete(NET_ACTIVE_WINDOW, dwm.root);

	XCloseDisplay(dwm.dpy);
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

	EEXIT("xrequest code=%d, error code=%d\n", ee->request_code, ee->error_code);

	return -1;
}

static int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee){
	EEXIT("another window manager is already running\n");

	return -1;
}

static void check_other_wm_running(void){
	xlib_set_error_handler(startup_xerror_hdlr);

	/* this causes an error if some other window manager is dwm.running */
	XSelectInput(dwm.dpy, DefaultRootWindow(dwm.dpy), SubstructureRedirectMask);

	xlib_sync();
	xlib_set_error_handler(0x0);
	xlib_sync();
}
