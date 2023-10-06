#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/XKBlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <xlib/client.h>
#include <config.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <xlib/monitor.h>
#include <core/statusbar.h>
#include <utils/timer.h>
#include <core/xevents.h>
#include <utils/log.h>


/* macros */
#define CLEANMASK(mask) (mask & ~(dwm.numlock_mask | LockMask) & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))


/* local/static prototypes */
static void buttonpress(XEvent *e);
static void configurerequest(XEvent *e);
static void configurenotify(XEvent *e);
static void destroynotify(XEvent *e);
static void expose(XEvent *e);
static void focusin(XEvent *e);
static void keypress(XEvent *e);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void propertynotify(XEvent *e);
static void unmapnotify(XEvent *e);

static int modifier_reset_hdlr(void);


/* static variables */
static void (*handler[LASTEvent])(XEvent*) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = 0x0,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
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

static unsigned char modifier_state = 0;
static int modifier_reset_timer = -1;
static cycle_callback_t cycle_complete = 0x0;


/* global functions */
int xlib_events_init(void){
	modifier_reset_timer = timer_init();

	if(modifier_reset_timer == -1)
		return -1;

	return event_add(modifier_reset_timer, modifier_reset_hdlr);
}

void xlib_cleanup(void){
	if(modifier_reset_timer != -1)
		close(modifier_reset_timer);
}

int xlib_events_hdlr(void){
	XEvent ev;


	while(XPending(dwm.dpy) != 0){
		if(XNextEvent(dwm.dpy, &ev))
			return -1;

		xlib_event_handle(&ev);
	}

	return 0;
}

void xlib_event_handle(XEvent *ev){
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

void key_cycle_start(cycle_callback_t complete){
	XkbStateRec state;


	XkbGetState(dwm.dpy, XkbUseCoreKbd, &state);

	if(state.mods == 0)
		return;

	key_cycle_complete();

	// the xlib KeyRelease event does not reliably report the release of modifier keys,
	// hence use a timer to reset the modifier state manually
	if(timer_set(modifier_reset_timer, 100) != 0)
		die("unable to start key-client_cycle timer\n");

	modifier_state = state.mods;
	cycle_complete = complete;
}

void key_cycle_complete(void){
	if(cycle_complete != 0x0)
		cycle_complete();

	modifier_state = 0;
	cycle_complete = 0x0;

	if(timer_set(modifier_reset_timer, 0) != 0)
		die("unable to stop key-client_cycle timer\n");
}

bool key_cycle_active(void){
	return (modifier_state != 0);
}


/* local functions */
static void buttonpress(XEvent *e){
	unsigned int i, x, click;
	action_arg_t arg = {0};
	client_t *c;
	XButtonPressedEvent *ev = &e->xbutton;


	click = ClkRootWin;

	/* client_focus monitor if necessary */
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
	else if((c = client_from_win(ev->window))){
		client_focus(c, true);
		XAllowEvents(dwm.dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
		statusbar_raise();
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


	if((c = client_from_win(ev->window))){
		geom = &c->geom;

		if(ev->value_mask & CWBorderWidth){
			geom->border_width = ev->border_width;
		}
		else{
			m = monitor_from_client(c);

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
				client_configure(c);

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

static void configurenotify(XEvent *e){
	XConfigureEvent *ev = &e->xconfigure;


	if(ev->window != dwm.root)
		return;

	dwm.screen_width = ev->width;
	dwm.screen_height = ev->height;

	monitor_discover();

	gfx_resize(dwm.gfx, dwm.screen_width, dwm.screen_height);
	XMoveResizeWindow(dwm.dpy, dwm.statusbar.win, dwm.mons->x, dwm.statusbar.y, dwm.mons->width, dwm.statusbar.height);
	layout_arrange();
}

static void destroynotify(XEvent *e){
	XDestroyWindowEvent *ev = &e->xdestroywindow;
	client_t *c;


	if((c = client_from_win(ev->window)))
		client_cleanup(c, 1);
}

static void expose(XEvent *e){
	XExposeEvent *ev = &e->xexpose;


	if(ev->window == dwm.statusbar.win && ev->count == 0)
		statusbar_update();
}

static void focusin(XEvent *e){
	/* there are some broken client_focus acquiring clients needing extra handling */
	XFocusChangeEvent *ev = &e->xfocus;


	if(dwm.focused && ev->window != dwm.focused->win)
		client_focus(dwm.focused, false);
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

	if(!client_from_win(ev->window))
		client_init(ev->window, &wa);
}

static void propertynotify(XEvent *e){
	XPropertyEvent *ev = &e->xproperty;
	client_t *c;
//	Window trans;


	if(ev->state == PropertyDelete)
		return;

	if((ev->window == dwm.root) && (ev->atom == XA_WM_NAME)){
		statusbar_update();
	}
	else if((c = client_from_win(ev->window))){
		switch(ev->atom){
		// TODO check if the following is still needed with isfloating being removed
//		case XA_WM_TRANSIENT_FOR:
//			if(!c->isfloating && (XGetTransientForHint(dwm.dpy, c->win, &trans)) && (c->isfloating = (client_from_win(trans)) != NULL))
//				layout_arrange(c->mon);
//			break;

		case XA_WM_NORMAL_HINTS:
			client_update_sizehints(c);
			break;

		case XA_WM_HINTS:
			client_update_wmhints(c);
			break;
		}
	}
}

static void unmapnotify(XEvent *e){
	XUnmapEvent *ev = &e->xunmap;
	client_t *c;


	if((c = client_from_win(ev->window))){
		if(ev->send_event)	client_set_state(c, WithdrawnState);
		else				client_cleanup(c, 0);
	}
}

static int modifier_reset_hdlr(void){
	uint64_t data;
	XkbStateRec state;


	read(modifier_reset_timer, &data, sizeof(data));

	XkbGetState(dwm.dpy, XkbUseCoreKbd, &state);
	modifier_state &= state.mods;

	if(modifier_state == 0)
		key_cycle_complete();

	return 0;
}
