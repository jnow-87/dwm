#include <client.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <events.h>
#include <layout.h>
#include <locale.h>
#include <monitor.h>
#include <signal.h>
#include <statusbar.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utils.h>
#include <version.h>
#include <layout.h>
#include <log.h>


/* local/static prototypes */
static void setup(void);
static void cleanup(void);
static void scan(void);
static void run(void);
static void check_other_wm_running(void);
static long getstate(Window w);


/* global variables */
dwm_t dwm = {
	.layout = layouts + 0,
	.running = 1,
	.numlock_mask = 0,
};


/* static variables */
static Window wmcheckwin;


/* global functions */
int main(int argc, char *argv[]){
	if(argc == 2 && !strcmp("-v", argv[1]))
		die("dwm-" VERSION);

	if(argc != 1)
		die("usage: dwm [-v]");

	setup();
	run();
	cleanup();

	// restart
	if(dwm.running < 0)
		execvp(argv[0], argv);

	return EXIT_SUCCESS;
}


/* local functions */
static void setup(void){
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
	dwm.drw = drw_create(dwm.dpy, dwm.screen, dwm.root, dwm.screen_width, dwm.screen_height);

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
	dwm.netatom[NetWMState] = XInternAtom(dwm.dpy, "_NET_WM_STATE", False);
	dwm.netatom[NetWMCheck] = XInternAtom(dwm.dpy, "_NET_SUPPORTING_WM_CHECK", False);
	dwm.netatom[NetWMFullscreen] = XInternAtom(dwm.dpy, "_NET_WM_STATE_FULLSCREEN", False);
	dwm.netatom[NetWMWindowType] = XInternAtom(dwm.dpy, "_NET_WM_WINDOW_TYPE", False);
	dwm.netatom[NetClientList] = XInternAtom(dwm.dpy, "_NET_CLIENT_LIST", False);

	/* init cursors */
	dwm.cursor[CurNormal] = drw_cur_create(dwm.drw, XC_left_ptr);
	dwm.cursor[CurResize] = drw_cur_create(dwm.drw, XC_sizing);
	dwm.cursor[CurMove] = drw_cur_create(dwm.drw, XC_fleur);

	/* init appearance */
	dwm.scheme = ecalloc(ncolors, sizeof(color_t*));

	for(int i=0; i<ncolors; i++)
		dwm.scheme[i] = drw_scm_create(dwm.drw, colors[i], 3);

	/* init bars */
	statusbar_init(dwm.drw->fonts->h + 2);
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

	focus(NULL);
	scan();
}

static void cleanup(void){
	action_arg_t a = {.ui = ~0};
	layout_t foo = { "", NULL };
	monitor_t *m;
	size_t i;


	action_view(&a);
	dwm.layout = &foo;

	for(m=dwm.mons; m; m=m->next){
		while(m->stack)
			unmanage(m->stack, 0);
	}

	XUngrabKey(dwm.dpy, AnyKey, AnyModifier, dwm.root);

	while(dwm.mons)
		cleanupmon(dwm.mons);

	for(i=0; i<CurLast; i++)
		drw_cur_free(dwm.drw, dwm.cursor[i]);

	for(i=0; i<ncolors; i++)
		free(dwm.scheme[i]);

	free(dwm.scheme);
	XDestroyWindow(dwm.dpy, wmcheckwin);
	drw_free(dwm.drw);
	XSync(dwm.dpy, False);
	XSetInputFocus(dwm.dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	XCloseDisplay(dwm.dpy);
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
			manage(childs[i], &wa);
	}

	for(unsigned int i=0; i<nchilds; i++){ /* now the transients */
		if(!XGetWindowAttributes(dwm.dpy, childs[i], &wa))
			continue;

		if(XGetTransientForHint(dwm.dpy, childs[i], &dummy) && (wa.map_state == IsViewable || getstate(childs[i]) == IconicState))
			manage(childs[i], &wa);
	}

	XFree(childs);
}

static void run(void){
	XEvent ev;


	/* main event loop */
	XSync(dwm.dpy, False);

	while(dwm.running > 0 && !XNextEvent(dwm.dpy, &ev))
		handle_event(&ev);
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
