#include <stdbool.h>
#include <stdlib.h>
#include <locale.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <core/dwm.h>
#include <xlib/xlib.h>
#include <xlib/gfx.h>
#include <utils/log.h>
#include <config.h>


/* local/static prototypes */
static int xerror_hdlr(Display *dpy, XErrorEvent *ee);
static int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee);
static void check_other_wm_running(void);


/* global functions */
int xlib_init(void){
	XSetWindowAttributes wa;
	Atom utf8string;


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

	dwm.gfx = gfx_create(dwm.screen_width, dwm.screen_height, colors, ncolors);

	if(dwm.gfx == 0x0)
		return STRERROR("creating grafix context");

	dwm.numlock_mask = input_get_numlock_mask();

	/* init atoms */
	utf8string = XInternAtom(dwm.dpy, "UTF8_STRING", False);
	dwm.wmatom[WMProtocols] = XInternAtom(dwm.dpy, "WM_PROTOCOLS", False);
	dwm.wmatom[WMDelete] = XInternAtom(dwm.dpy, "WM_DELETE_WINDOW", False);
	dwm.wmatom[WMState] = XInternAtom(dwm.dpy, "WM_STATE", False);
	dwm.wmatom[WMTakeFocus] = XInternAtom(dwm.dpy, "WM_TAKE_FOCUS", False);

	dwm.netatom[NetActiveWindow] = XInternAtom(dwm.dpy, "_NET_ACTIVE_WINDOW", False);
	dwm.netatom[NetSupported] = XInternAtom(dwm.dpy, "_NET_SUPPORTED", False);
	dwm.netatom[NetWMName] = XInternAtom(dwm.dpy, "_NET_WM_NAME", False);
	dwm.netatom[NetWMCheck] = XInternAtom(dwm.dpy, "_NET_SUPPORTING_WM_CHECK", False);
	dwm.netatom[NetClientList] = XInternAtom(dwm.dpy, "_NET_CLIENT_LIST", False);

	/* supporting window for NetWMCheck */
	// this is a requirement to indicate a conforming window manager, cf.
	// https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html#idm45771211439200
	dwm.wmcheck = XCreateSimpleWindow(dwm.dpy, dwm.root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dwm.dpy, dwm.wmcheck, dwm.netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&dwm.wmcheck, 1);
	XChangeProperty(dwm.dpy, dwm.wmcheck, dwm.netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm", 3);
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&dwm.wmcheck, 1);

	/* extended window manager hints (EWMH) support per view */
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)dwm.netatom, NetLast);
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList]);

	/* select events */
	wa.cursor = dwm.gfx->cursors[CurNormal];

	// TODO
	// 	the list doesn't seem correct, cf. notes
	wa.event_mask = SubstructureRedirectMask
				  | SubstructureNotifyMask
				  | ButtonPressMask
				  | PointerMotionMask
				  | LeaveWindowMask
				  | StructureNotifyMask
				  | PropertyChangeMask
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
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);

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
