/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the client_focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys mappings and other configurations are defined in config.h.
 *
 * To understand everything else, start reading main().
 */

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <config/config.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <xlib/window.h>
#include <core/scheme.h>
#include <config.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <utils/math.h>
#include <xlib/xinerama.h>
#include <stdlib.h>
#include <xlib/window.h>
#include <config.h>
#include <config/config.h>
#include <core/dwm.h>
#include <core/xevents.h>
#include <core/layout.h>
#include <locale.h>
#include <core/monitor.h>
#include <signal.h>
#include <core/statusbar.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <utils/math.h>
#include <version.h>
#include <core/layout.h>
#include <core/tags.h>
#include <xlib/input.h>
#include <xlib/xlib.h>
#include <core/keys.h>
#include <utils/log.h>




/* local/static prototypes */
static void scan(void);
static void check_other_wm_running(void);
static long getstate(Window w);
static int xevent_hdlr(void);
static int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee);


/* static variables */
static Window wmcheckwin;


/* global functions */
int dwm_setup(void){
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction sa;


	if(log_init(CONFIG_LOG_FILE, true) != 0)
		dwm_die("dwm: cannot open log-file %s\n", CONFIG_LOG_FILE);

	DEBUG("dwm hello\n");

	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);

	if(!(dwm.dpy = XOpenDisplay(NULL)))
		dwm_die("dwm: cannot open display");

	check_other_wm_running();

	dwm.event_fd = epoll_create(1);

	if(dwm.event_fd == -1)
		dwm_die("unable to init event loop\n");

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, NULL);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while(waitpid(-1, NULL, WNOHANG) > 0);

	/* init screen */
	dwm.screen = DefaultScreen(dwm.dpy);
	dwm.screen_width = DisplayWidth(dwm.dpy, dwm.screen);
	dwm.screen_height = DisplayHeight(dwm.dpy, dwm.screen);
	dwm.root = RootWindow(dwm.dpy, dwm.screen);
	dwm.gfx = gfx_create(dwm.dpy, dwm.screen, dwm.root, dwm.screen_width, dwm.screen_height);
	dwm.numlock_mask = input_get_numlock_mask();

	if(dwm.gfx == 0x0)
		return STRERROR("error creating grafix context");

	if(dwm_hdlr_add(ConnectionNumber(dwm.dpy), xevent_hdlr))
		dwm_die("error adding xlib event handler\n");

	keys_init();
	monitor_discover();

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

	/* init cursors */
	dwm.cursor[CurNormal] = gfx_cur_create(dwm.gfx, XC_left_ptr);
	dwm.cursor[CurResize] = gfx_cur_create(dwm.gfx, XC_sizing);
	dwm.cursor[CurMove] = gfx_cur_create(dwm.gfx, XC_fleur);

	/* init appearance */
	dwm.scheme = calloc(ncolors, sizeof(color_t*));

	if(dwm.scheme == 0x0)
		return STRERROR("cannot allocate scheme");

	for(int i=0; i<ncolors; i++)
		dwm.scheme[i] = gfx_scm_create(dwm.gfx, colors[i], 3);

	/* init bars */
	statusbar_init(dwm.gfx->fonts->h + 2);
	statusbar_update();

	/* supporting window for NetWMCheck */
	// this is a requirement to indicate a conforming window manager, cf.
	// https://specifications.freedesktop.org/wm-spec/wm-spec-latest.html#idm45771211439200
	wmcheckwin = XCreateSimpleWindow(dwm.dpy, dwm.root, 0, 0, 1, 1, 0, 0, 0);
	XChangeProperty(dwm.dpy, wmcheckwin, dwm.netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);
	XChangeProperty(dwm.dpy, wmcheckwin, dwm.netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm", 3);
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);

	/* extended window manager hints (EWMH) support per view */
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)dwm.netatom, NetLast);
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList]);

	/* select events */
	wa.cursor = dwm.cursor[CurNormal]->cursor;

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
	input_register_key_mappings(keys, nkeys);

	scan();
	layout_arrange();

	xlib_sync();

	return 0;
}

void dwm_cleanup(void){
	layout_t foo = { "", NULL };
	size_t i;


	tags_set(&dwm.tag_mask, ~0);
	dwm.layout = &foo;

	while(dwm.stack)
		client_cleanup(dwm.stack, false);

	XUngrabKey(dwm.dpy, AnyKey, AnyModifier, dwm.root);

	while(dwm.mons){
		monitor_destroy(dwm.mons);
	}

	for(i=0; i<CurLast; i++)
		gfx_cur_free(dwm.gfx, dwm.cursor[i]);

	for(i=0; i<ncolors; i++)
		free(dwm.scheme[i]);

	free(dwm.scheme);
	XDestroyWindow(dwm.dpy, wmcheckwin);
	gfx_free(dwm.gfx);
	xlib_sync();
	XSetInputFocus(dwm.dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	XCloseDisplay(dwm.dpy);

	keys_cleanup();
	close(dwm.event_fd);
	log_cleanup();
}

void dwm_run(void){
	int n;
	struct epoll_event evts[2];


	while(dwm.running > 0){
		// handle xevents that occured after the last epoll notification
		// and haven't been processed yet
		xevents_handle_events();

		n = epoll_wait(dwm.event_fd, evts, LENGTH(evts), -1);

		for(; n>0; n--){
			if(((event_hdlr_t)evts[n - 1].data.ptr)() != 0)
				return;
		}
	}
}

void dwm_die(char const *fmt, ...){
	va_list ap;


	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if(fmt[0] && fmt[strlen(fmt) - 1] == ':'){
		fputc(' ', stderr);
		perror(NULL);
	}
	else
		fputc('\n', stderr);

	exit(1);
}

int dwm_hdlr_add(int fd, event_hdlr_t hdlr){
	struct epoll_event ev;


	ev.events = EPOLLIN;
	ev.data.ptr = hdlr;

	return epoll_ctl(dwm.event_fd, EPOLL_CTL_ADD, fd, &ev);
}


/* local functions */
static void scan(void){
	unsigned int nchilds;
	window_t dummy;
	window_t *childs;
	win_attr_t attr;


	if(!XQueryTree(dwm.dpy, dwm.root, &dummy, &dummy, &childs, &nchilds) || childs == 0x0)
		return;

	for(unsigned int i=0; i<nchilds; i++){
		if(win_get_attr(childs[i], &attr) != 0 || attr.override_redirect || XGetTransientForHint(dwm.dpy, childs[i], &dummy))
			continue;

		if(attr.map_state == IsViewable || getstate(childs[i]) == IconicState)
			client_init(childs[i], &attr);
	}

	for(unsigned int i=0; i<nchilds; i++){ /* now the transients */
		if(win_get_attr(childs[i], &attr) != 0)
			continue;

		if(XGetTransientForHint(dwm.dpy, childs[i], &dummy) && (attr.map_state == IsViewable || getstate(childs[i]) == IconicState))
			client_init(childs[i], &attr);
	}

	XFree(childs);
}

static void check_other_wm_running(void){
	xlib_set_error_handler(startup_xerror_hdlr);

	/* this causes an error if some other window manager is dwm.running */
	XSelectInput(dwm.dpy, DefaultRootWindow(dwm.dpy), SubstructureRedirectMask);
	xlib_sync();
	xlib_set_error_handler(0x0);
	xlib_sync();
}

static long getstate(Window w){
	int format;
	long result = -1;
	unsigned char *p = NULL;
	unsigned long n, extra;
	Atom real;


	if(XGetWindowProperty(dwm.dpy, w, dwm.wmatom[WMState], 0L, 2L, False, dwm.wmatom[WMState], &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;

	if(n != 0)
		result = *p;

	XFree(p);

	return result;
}

static int xevent_hdlr(void){
	return xevents_handle_events();
}

static int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee){
	// startup Error handler to check if another window manager is already running
	dwm_die("dwm: another window manager is already running");

	return -1;
}
