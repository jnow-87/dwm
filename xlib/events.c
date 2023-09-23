#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <client.h>
#include <config.h>
#include <dwm.h>
#include <layout.h>
#include <monitor.h>
#include <statusbar.h>


/* macros */
#define CLEANMASK(mask) (mask & ~(dwm.numlock_mask | LockMask) & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))


/* local/static prototypes */
static void buttonpress(XEvent *e);
static void clientmessage(XEvent *e);
static void configurenotify(XEvent *e);
static void configurerequest(XEvent *e);
static void destroynotify(XEvent *e);
static void expose(XEvent *e);
static void focusin(XEvent *e);
static void keypress(XEvent *e);
static void mappingnotify(XEvent *e);
static void maprequest(XEvent *e);
static void motionnotify(XEvent *e);
static void propertynotify(XEvent *e);
static void unmapnotify(XEvent *e);


/* static variables */
static void (*handler[LASTEvent])(XEvent*) = {
	[ButtonPress] = buttonpress,
	[ClientMessage] = clientmessage,
	[ConfigureRequest] = configurerequest,
	[ConfigureNotify] = configurenotify,
	[DestroyNotify] = destroynotify,
	[EnterNotify] = 0x0,
	[Expose] = expose,
	[FocusIn] = focusin,
	[KeyPress] = keypress,
	[MappingNotify] = mappingnotify,
	[MapRequest] = maprequest,
	[MotionNotify] = motionnotify,
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
	monitor_t *m;
	XButtonPressedEvent *ev = &e->xbutton;


	click = ClkRootWin;

	/* focus monitor if necessary */
	if((m = wintomon(ev->window)) && m != dwm.selmon){
		unfocus(dwm.selmon->sel, 1);
		dwm.selmon = m;
		focus(NULL);
	}

	if(ev->window == dwm.selmon->barwin){
		i = x = 0;

		do{
			x += TEXTW(tags[i]);
		} while(ev->x >= x && ++i < ntags);

		if(i < ntags){
			click = ClkTagBar;
			arg.ui = 1 << i;
		}
		else if(ev->x < x + TEXTW(dwm.selmon->ltsymbol)){
			click = ClkLtSymbol;
		}
		else if(ev->x > dwm.selmon->ww - (int)TEXTW(stext)){
			click = ClkStatusText;
		}
		else
			click = ClkWinTitle;
	}
	else if((c = wintoclient(ev->window))){
		focus(c);
		restack(dwm.selmon);
		XAllowEvents(dwm.dpy, ReplayPointer, CurrentTime);
		click = ClkClientWin;
	}

	for(i=0; i<nbuttons; i++){
		if(click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
			&& CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
			buttons[i].func(click == ClkTagBar && buttons[i].arg.i == 0 ? &arg : &buttons[i].arg);
	}
}

static void clientmessage(XEvent *e){
	XClientMessageEvent *cme = &e->xclient;
	client_t *c = wintoclient(cme->window);


	if(!c)
		return;

	if(cme->message_type == dwm.netatom[NetWMState]){
		if(cme->data.l[1] == dwm.netatom[NetWMFullscreen] || cme->data.l[2] == dwm.netatom[NetWMFullscreen])
			setfullscreen(c, (cme->data.l[0] == 1 /* _NET_WM_STATE_ADD    */ || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */ && !c->isfullscreen)));
	}
	else if(cme->message_type == dwm.netatom[NetActiveWindow]){
		if(c != dwm.selmon->sel && !c->isurgent)
			seturgent(c, 1);
	}
}

static void configurenotify(XEvent *e){
	monitor_t *m;
	client_t *c;
	XConfigureEvent *ev = &e->xconfigure;
	int dirty;


	/* TODO: updategeom handling sucks, needs to be simplified */
	if(ev->window == dwm.root){
		dirty = (dwm.screen_width != ev->width || dwm.screen_height != ev->height);
		dwm.screen_width = ev->width;
		dwm.screen_height = ev->height;

		if(updategeom() || dirty){
			drw_resize(dwm.drw, dwm.screen_width, dwm.statusbar_height);
			updatebars();

			for(m=dwm.mons; m; m=m->next){
				for(c=m->clients; c; c=c->next){
					if(c->isfullscreen)
						resizeclient(c, m->mx, m->my, m->mw, m->mh);
				}

				XMoveResizeWindow(dwm.dpy, m->barwin, m->wx, m->by, m->ww, dwm.statusbar_height);
			}

			focus(NULL);
			arrange(NULL);
		}
	}
}

static void configurerequest(XEvent *e){
	client_t *c;
	monitor_t *m;
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	XWindowChanges wc;


	if((c = wintoclient(ev->window))){
		if(ev->value_mask & CWBorderWidth){
			c->bw = ev->border_width;
		}
		else if(c->isfloating || !dwm.selmon->lt[dwm.selmon->sellt]->arrange){
			m = c->mon;

			if(ev->value_mask & CWX){
				c->oldx = c->x;
				c->x = m->mx + ev->x;
			}

			if(ev->value_mask & CWY){
				c->oldy = c->y;
				c->y = m->my + ev->y;
			}

			if(ev->value_mask & CWWidth){
				c->oldw = c->w;
				c->w = ev->width;
			}

			if(ev->value_mask & CWHeight){
				c->oldh = c->h;
				c->h = ev->height;
			}

			if((c->x + c->w) > m->mx + m->mw && c->isfloating)
				c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2);	/* center in x direction */

			if((c->y + c->h) > m->my + m->mh && c->isfloating)
				c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */

			if((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight)))
				configure(c);

			if(ISVISIBLE(c))
				XMoveResizeWindow(dwm.dpy, c->win, c->x, c->y, c->w, c->h);
		}
		else
			configure(c);
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
	client_t *c;
	XDestroyWindowEvent *ev = &e->xdestroywindow;


	if((c = wintoclient(ev->window)))
		unmanage(c, 1);
}

static void expose(XEvent *e){
	monitor_t *m;
	XExposeEvent *ev = &e->xexpose;


	if(ev->count == 0 && (m = wintomon(ev->window)))
		drawbar(m);
}

static void focusin(XEvent *e){
	/* there are some broken focus acquiring clients needing extra handling */
	XFocusChangeEvent *ev = &e->xfocus;


	if(dwm.selmon->sel && ev->window != dwm.selmon->sel->win)
		setfocus(dwm.selmon->sel);
}

static void keypress(XEvent *e){
	unsigned int i;
	KeySym keysym;
	XKeyEvent *ev;


	ev = &e->xkey;
	keysym = XKeycodeToKeysym(dwm.dpy, (KeyCode)ev->keycode, 0);
	for(i=0; i<nkeys; i++){
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
	static XWindowAttributes wa;
	XMapRequestEvent *ev = &e->xmaprequest;


	if(!XGetWindowAttributes(dwm.dpy, ev->window, &wa) || wa.override_redirect)
		return;

	if(!wintoclient(ev->window))
		manage(ev->window, &wa);
}

static void motionnotify(XEvent *e){
	static monitor_t *mon = NULL;
	monitor_t *m;
	XMotionEvent *ev = &e->xmotion;


	if(ev->window != dwm.root)
		return;

	if((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon){
		unfocus(dwm.selmon->sel, 1);
		dwm.selmon = m;
		focus(NULL);
	}

	mon = m;
}

static void propertynotify(XEvent *e){
	client_t *c;
	Window trans;
	XPropertyEvent *ev = &e->xproperty;


	if((ev->window == dwm.root) && (ev->atom == XA_WM_NAME)){
		updatestatus();
	}
	else if(ev->state == PropertyDelete){
		return; /* ignore */
	}
	else if((c = wintoclient(ev->window))){
		switch(ev->atom){
		default: break;
		case XA_WM_TRANSIENT_FOR:
			if(!c->isfloating && (XGetTransientForHint(dwm.dpy, c->win, &trans)) && (c->isfloating = (wintoclient(trans)) != NULL))
				arrange(c->mon);
			break;

		case XA_WM_NORMAL_HINTS:
			c->hintsvalid = 0;
			break;

		case XA_WM_HINTS:
			updatewmhints(c);
			drawbars();
			break;

		}

		if(ev->atom == XA_WM_NAME || ev->atom == dwm.netatom[NetWMName]){
			updatetitle(c);

			if(c == c->mon->sel)
				drawbar(c->mon);
		}

		if(ev->atom == dwm.netatom[NetWMWindowType])
			updatewindowtype(c);
	}
}

static void unmapnotify(XEvent *e){
	client_t *c;
	XUnmapEvent *ev = &e->xunmap;


	if((c = wintoclient(ev->window))){
		if(ev->send_event)	setclientstate(c, WithdrawnState);
		else				unmanage(c, 0);
	}
}
