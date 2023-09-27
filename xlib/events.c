#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <client.h>
#include <config.h>
#include <dwm.h>
#include <layout.h>
#include <monitor.h>
#include <statusbar.h>
#include <log.h>


/* macros */
#define CLEANMASK(mask) (mask & ~(dwm.numlock_mask | LockMask) & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))


/* local/static prototypes */
static void buttonpress(XEvent *e);
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void expose(XEvent *e);
static void focusin(XEvent *e);
static void keypress(XEvent *e);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void propertynotify(XEvent *e);
static void unmapnotify(XEvent *e);


/* static variables */
static void (*handler[LASTEvent])(XEvent*) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = 0x0,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = 0x0,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = 0x0,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = 0x0,
	[PropertyNotify] = propertynotify,
	[UnmapNotify] = unmapnotify
};


/* global functions */
void handle_event(XEvent *ev){
	if(handler[ev->type] != 0x0)
		handler[ev->type](ev);
}

int xerror_hdlr(Display *dpy, XErrorEvent *ee){
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

	fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n", ee->request_code, ee->error_code);

	return dwm.xlib_xerror_hdlr(dwm.dpy, ee); /* may call exit */
}

int startup_xerror_hdlr(Display *dpy, XErrorEvent *ee){
	// startup Error handler to check if another window manager is already running
	die("dwm: another window manager is already running");

	return -1;
}

int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee){
	return 0;
}


/* local functions */
static void buttonpress(XEvent *e){
	unsigned int i, x, click;
	action_arg_t arg = {0};
	client_t *c;
	XButtonPressedEvent *ev = &e->xbutton;


	click = ClkRootWin;

	/* focus monitor if necessary */
	if(ev->window == dwm.statusbar.win){
		i = x = 0;

		do{
			x += TEXTW(tags[i]);
		} while(ev->x >= x && ++i < ntags);

		if(i < ntags){
			click = ClkTagBar;
			arg.ui = 1 << i;
		}
		else if(ev->x < x + TEXTW(dwm.layout->symbol)){
			click = ClkLtSymbol;
		}
		else if(ev->x > dwm.mons->width - (int)TEXTW(dwm.statusbar.status)){
			click = ClkStatusText;
		}
		else
			click = ClkWinTitle;
	}
	else if((c = wintoclient(ev->window))){
		focus(c);
		restack(dwm.mons);
		XAllowEvents(dwm.dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
		statusbar_draw();
	}

	for(i=0; i<nbuttons; i++){
		if(click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
	}
}

static void configurerequest(XEvent *e){
	client_t *c;
	client_geom_t *geom;
	monitor_t *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;


	if((c = wintoclient(ev->window))){
		geom = &c->geom;

		if(ev->value_mask & CWBorderWidth){
			geom->border_width = ev->border_width;
		}
		else{
			m = c->mon;

			if(ev->value_mask & CWX){
				c->geom_store.x = geom->x;
				geom->x = m->x + ev->x;
			}

			if(ev->value_mask & CWY){
				c->geom_store.y = geom->y;
				geom->y = m->y + ev->y;
			}

			if(ev->value_mask & CWWidth){
				c->geom_store.width = geom->width;
				geom->width = ev->width;
			}

			if(ev->value_mask & CWHeight){
				c->geom_store.height = geom->height;
				geom->height = ev->height;
			}

			if((geom->x + geom->width) > m->x + m->width)
				geom->x = m->x + (m->width / 2 - WIDTH(c) / 2);	/* center in x direction */

			if((geom->y + geom->height) > m->y + m->height)
				geom->y = m->y + (m->height / 2 - HEIGHT(c) / 2); /* center in y direction */

			if((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight)))
				configure(c);

			if(ISVISIBLE(c))
				XMoveResizeWindow(dwm.dpy, c->win, geom->x, geom->y, geom->width, geom->height);
		}
	}
	else{
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;
		XConfigureWindow(dwm.dpy, ev->window, ev->value_mask, &wc);
	}

	XSync(dwm.dpy, False);
}

static void destroynotify(XEvent *e){
	XDestroyWindowEvent *ev = &e->xdestroywindow;
	client_t *c;


	if((c = wintoclient(ev->window)))
		unmanage(c, 1);
}

static void expose(XEvent *e){
	XExposeEvent *ev = &e->xexpose;
	monitor_t *m;


	if(ev->count == 0 && (m = wintomon(ev->window)))
		statusbar_draw();
}

static void focusin(XEvent *e){
	/* there are some broken focus acquiring clients needing extra handling */
	XFocusChangeEvent *ev = &e->xfocus;


	if(dwm.mons->sel && ev->window != dwm.mons->sel->win)
		setfocus(dwm.mons->sel);
}

static void keypress(XEvent *e){
	XKeyEvent *ev = &e->xkey;
	KeySym keysym;


	keysym = XKeycodeToKeysym(dwm.dpy, (KeyCode)ev->keycode, 0);

	for(unsigned int i=0; i<nkeys; i++){
		if(keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func)
			keys[i].func(&(keys[i].arg));
	}
}

static void mappingnotify(XEvent *e){
	XMappingEvent *ev = &e->xmapping;


	XRefreshKeyboardMapping(ev);

	if(ev->request == MappingKeyboard)
		grabkeys();
}

static void maprequest(XEvent *e){
	XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;


	if(!XGetWindowAttributes(dwm.dpy, ev->window, &wa) || wa.override_redirect)
		return;

	if(!wintoclient(ev->window))
		manage(ev->window, &wa);
}

static void propertynotify(XEvent *e){
	XPropertyEvent *ev = &e->xproperty;
	client_t *c;
//	Window trans;


	if(ev->state == PropertyDelete)
		return;

	if((ev->window == dwm.root) && (ev->atom == XA_WM_NAME)){
		statusbar_update();
		statusbar_draw();
	}
	else if((c = wintoclient(ev->window))){
		switch(ev->atom){
		// TODO check if the following is still needed with isfloating being removed
//		case XA_WM_TRANSIENT_FOR:
//			if(!c->isfloating && (XGetTransientForHint(dwm.dpy, c->win, &trans)) && (c->isfloating = (wintoclient(trans)) != NULL))
//				arrange(c->mon);
//			break;

		case XA_WM_NORMAL_HINTS:
			updatesizehints(c);
			break;

		case XA_WM_HINTS:
			updatewmhints(c);
			statusbar_draw();
			break;
		}
	}
}

static void unmapnotify(XEvent *e){
	XUnmapEvent *ev = &e->xunmap;
	client_t *c;


	if((c = wintoclient(ev->window))){
		if(ev->send_event)	setclientstate(c, WithdrawnState);
		else				unmanage(c, 0);
	}
}
