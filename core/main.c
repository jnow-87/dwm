#include <stdlib.h>
#include <xlib/client.h>
#include <config.h>
#include <config/config.h>
#include <core/dwm.h>
#include <core/xevents.h>
#include <core/layout.h>
#include <locale.h>
#include <xlib/monitor.h>
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
#include <utils/log.h>


/* local/static prototypes */
static int setup(void);
static void cleanup(void);
static void scan(void);
static void run(void);
static void check_other_wm_running(void);
static long getstate(Window w);


/* global variables */
dwm_t dwm = {
	.stack = 0x0,
	.layout = layouts + 0,
	.running = 1,
	.numlock_mask = 0,
	.tag_mask = 1,
};


/* static variables */
static Window wmcheckwin;


/* global functions */
int main(int argc, char *argv[]){
	if(argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-" VERSION);

	if(argc != 1)
		die("usage: dwm [-v]");

	if(setup() != 0)
		die("setup failed\n");

	run();
	cleanup();

	// restart
	if(dwm.running < 0)
		execvp(argv[0], argv);

	return EXIT_SUCCESS;
}


/* local functions */
static int setup(void){
	XSetWindowAttributes wa;
	Atom utf8string;
	struct sigaction sa;


	if(log_init(CONFIG_LOG_FILE, true) != 0)
		die("dwm: cannot open log-file %s\n", CONFIG_LOG_FILE);

	DEBUG("dwm hello\n");

	if(!setlocale(LC_CTYPE, "") || !XSupportsLocale())
		fputs("warning: no locale support\n", stderr);

	if(!(dwm.dpy = XOpenDisplay(NULL)))
		die("dwm: cannot open display");

	check_other_wm_running();

	dwm.event_fd = epoll_create(1);

	if(dwm.event_fd == -1)
		die("unable to init event loop\n");

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

	if(dwm.gfx == 0x0)
		return STRERROR("error creating grafix context");

	if(event_add(ConnectionNumber(dwm.dpy), xlib_events_hdlr))
		die("error adding xlib event handler\n");

	xlib_events_init();
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
	grabkeys();

	scan();

	XSync(dwm.dpy, False);

	return 0;
}

static void cleanup(void){
	layout_t foo = { "", NULL };
	size_t i;


	tags_set(&dwm.tag_mask, ~0);
	dwm.layout = &foo;

	while(dwm.stack)
		client_cleanup(dwm.stack, 0);

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
	XSync(dwm.dpy, False);
	XSetInputFocus(dwm.dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	XCloseDisplay(dwm.dpy);

	xlib_cleanup();
	close(dwm.event_fd);
	log_cleanup();
}

static void scan(void){
	unsigned int nchilds;
	Window dummy;
	Window *childs;
	XWindowAttributes wa;


	if(!XQueryTree(dwm.dpy, dwm.root, &dummy, &dummy, &childs, &nchilds) || childs == 0x0)
		return;

	for(unsigned int i=0; i<nchilds; i++){
		if(!XGetWindowAttributes(dwm.dpy, childs[i], &wa) || wa.override_redirect || XGetTransientForHint(dwm.dpy, childs[i], &dummy))
			continue;

		if(wa.map_state == IsViewable || getstate(childs[i]) == IconicState)
			client_init(childs[i], &wa);
	}

	for(unsigned int i=0; i<nchilds; i++){ /* now the transients */
		if(!XGetWindowAttributes(dwm.dpy, childs[i], &wa))
			continue;

		if(XGetTransientForHint(dwm.dpy, childs[i], &dummy) && (wa.map_state == IsViewable || getstate(childs[i]) == IconicState))
			client_init(childs[i], &wa);
	}

	XFree(childs);
}

static void run(void){
	int n;
	struct epoll_event evts[2];


	while(dwm.running > 0){
		xlib_events_hdlr();
		n = epoll_wait(dwm.event_fd, evts, LENGTH(evts), -1);

		for(; n>0; n--){
			if(((event_hdlr_t)evts[n - 1].data.ptr)() != 0)
				return;
		}
	}
}

static void check_other_wm_running(void){
	dwm.xlib_xerror_hdlr = XSetErrorHandler(startup_xerror_hdlr);

	/* this causes an error if some other window manager is dwm.running */
	XSelectInput(dwm.dpy, DefaultRootWindow(dwm.dpy), SubstructureRedirectMask);
	XSync(dwm.dpy, False);
	XSetErrorHandler(xerror_hdlr);
	XSync(dwm.dpy, False);
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
