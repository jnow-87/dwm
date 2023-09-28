#include <X11/Xatom.h>
#include <client.h>
#include <colors.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <atoms.h>
#include <events.h>
#include <layout.h>
#include <statusbar.h>
#include <utils.h>


/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)


/* local/static prototypes */
static void updateclientlist();
static int applysizehints(client_t *c, int *x, int *y, int *w, int *h, int interact);
static void detach(client_t *c);
static Atom getatomprop(client_t *c, Atom prop);
static void grabbuttons(client_t *c, int focused);


/* global functions */
client_t *wintoclient(Window w){
	client_t *c;
	monitor_t *m;


	for(m=dwm.mons; m; m=m->next){
		for(c=m->clients; c; c=c->next){
			if(c->win == w)
				return c;
		}
	}

	return NULL;
}

void manage(Window w, XWindowAttributes *wa){
	client_t *c, *t = NULL;
	Window trans = None;
	client_geom_t *geom;
	XWindowChanges wc;


	c = ecalloc(1, sizeof(client_t));
	c->win = w;
	geom = &c->geom;

	/* init client struct and configure the xlib client accordingly */
	geom->x = wa->x;
	geom->y = wa->y;
	geom->width = wa->width;
	geom->height = wa->height;
	geom->border_width = wa->border_width;

	c->geom_store = *geom;

	if(XGetTransientForHint(dwm.dpy, w, &trans) && (t = wintoclient(trans))){
		c->mon = t->mon;
		c->tags = t->tags;
	}
	else{
		c->mon = dwm.mons;
		c->tags = dwm.tag_mask;
	}

	if(geom->x + WIDTH(c) > c->mon->x + c->mon->width)
		geom->x = c->mon->x + c->mon->width - WIDTH(c);

	if(geom->y + HEIGHT(c) > c->mon->y + c->mon->height)
		geom->y = c->mon->y + c->mon->height - HEIGHT(c);

	geom->x = MAX(geom->x, c->mon->x);
	geom->y = MAX(geom->y, c->mon->y);
	geom->border_width = CONFIG_BORDER_PIXEL;

	wc.border_width = geom->border_width;
	XConfigureWindow(dwm.dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dwm.dpy, w, dwm.scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	updatesizehints(c);
	updatewmhints(c);
	XSelectInput(dwm.dpy, w, FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grabbuttons(c, 0);

	XRaiseWindow(dwm.dpy, c->win);

	/* add client to monitor lists */
	attach(c);
	attachstack(c);

	// add client to xserver client list for the window manager
	// TODO make this part of attach
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);

	XMoveResizeWindow(dwm.dpy, c->win, geom->x + 2 * dwm.screen_width, geom->y, geom->width, geom->height); /* some windows require this */
	setclientstate(c, NormalState);

	if(c->mon == dwm.mons)
		unfocus(dwm.mons->sel, 0);

	c->mon->sel = c;
	XMapWindow(dwm.dpy, c->win);
	focus(NULL);

	// TODO move arrange out of the function to avoid calling it multiple times during startup
	arrange(c->mon);
}

void unmanage(client_t *c, int destroyed){
	monitor_t *m = c->mon;
	XWindowChanges wc;


	detach(c);
	detachstack(c);

	if(!destroyed){
		wc.border_width = c->geom_store.border_width;
		XGrabServer(dwm.dpy); /* avoid race conditions */
		XSetErrorHandler(dummy_xerror_hdlr);
		XSelectInput(dwm.dpy, c->win, NoEventMask);
		XConfigureWindow(dwm.dpy, c->win, CWBorderWidth, &wc); /* restore border */
		XUngrabButton(dwm.dpy, AnyButton, AnyModifier, c->win);
		setclientstate(c, WithdrawnState);
		XSync(dwm.dpy, False);
		XSetErrorHandler(xerror_hdlr);
		XUngrabServer(dwm.dpy);
	}

	free(c);
	focus(NULL);
	updateclientlist();
	arrange(m);
}

void killclient(Window win){
	XGrabServer(dwm.dpy);
	XSetErrorHandler(dummy_xerror_hdlr);
	XSetCloseDownMode(dwm.dpy, DestroyAll);
	XKillClient(dwm.dpy, dwm.mons->sel->win);
	XSync(dwm.dpy, False);
	XSetErrorHandler(xerror_hdlr);
	XUngrabServer(dwm.dpy);
}

void configure(client_t *c){
	client_geom_t *geom = &c->geom;
	XConfigureEvent ce;


	ce.type = ConfigureNotify;
	ce.display = dwm.dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = geom->x;
	ce.y = geom->y;
	ce.width = geom->width;
	ce.height = geom->height;
	ce.border_width = geom->border_width;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dwm.dpy, c->win, False, StructureNotifyMask, (XEvent*)&ce);
}

void attach(client_t *c){
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void attachstack(client_t *c){
	c->stack_next = c->mon->stack;
	c->mon->stack = c;
}

void detachstack(client_t *c){
	client_t **tc, *t;


	for(tc=&c->mon->stack; *tc && *tc!=c; tc=&(*tc)->stack_next);

	*tc = c->stack_next;

	if(c == c->mon->sel){
		for(t=c->mon->stack; t && !ISVISIBLE(t); t=t->stack_next);

		c->mon->sel = t;
	}
}

void focus(client_t *c){
	if(!c || !ISVISIBLE(c))
		for(c=dwm.mons->stack; c && !ISVISIBLE(c); c=c->stack_next);

	if(dwm.mons->sel && dwm.mons->sel != c)
		unfocus(dwm.mons->sel, 0);

	if(c){
		detachstack(c);
		attachstack(c);
		grabbuttons(c, 1);
		XSetWindowBorder(dwm.dpy, c->win, dwm.scheme[SchemeSel][ColBorder].pixel);
		setfocus(c);
	}
	else{
		XSetInputFocus(dwm.dpy, dwm.root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	}

	dwm.mons->sel = c;
	statusbar_draw();
}

void unfocus(client_t *c, int setfocus){
	if(!c)
		return;

	grabbuttons(c, 0);
	XSetWindowBorder(dwm.dpy, c->win, dwm.scheme[SchemeNorm][ColBorder].pixel);

	if(setfocus){
		XSetInputFocus(dwm.dpy, dwm.root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	}
}

void showhide(client_t *c){
	client_geom_t *geom;


	if(!c)
		return;

	geom = &c->geom;

	if(ISVISIBLE(c)){
		/* show clients top down */
		XMoveWindow(dwm.dpy, c->win, geom->x, geom->y);
		resize(c, geom->x, geom->y, geom->width, geom->height, 0);
		showhide(c->stack_next);
	}
	else{
		/* hide clients bottom up */
		showhide(c->stack_next);
		XMoveWindow(dwm.dpy, c->win, WIDTH(c) * -2, geom->y);
	}
}

void sendmon(client_t *c, monitor_t *m){
	if(c->mon == m)
		return;

	unfocus(c, 1);
	detach(c);
	detachstack(c);

	c->mon = m;

	attach(c);
	attachstack(c);
	focus(NULL);
	arrange(NULL);
}

void resize(client_t *c, int x, int y, int w, int h, int interact){
	if(applysizehints(c, &x, &y, &w, &h, interact))
		resizeclient(c, x, y, w, h);
}

void resizeclient(client_t *c, int x, int y, int w, int h){
	client_geom_t *geom = &c->geom;
	XWindowChanges wc;


	c->geom_store = *geom;
	geom->x = wc.x = x;
	geom->y = wc.y = y;
	geom->width = wc.width = w;
	geom->height = wc.height = h;

	wc.border_width = geom->border_width;

	XConfigureWindow(dwm.dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	configure(c);
	XSync(dwm.dpy, False);
}

void setclientstate(client_t *c, long state){
	long data[] = {state, None};


	XChangeProperty(dwm.dpy, c->win, dwm.wmatom[WMState], dwm.wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

void setfocus(client_t *c){
	if(!c->hints.never_focus){
		XSetInputFocus(dwm.dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(c->win), 1);
	}

	sendevent(c, dwm.wmatom[WMTakeFocus]);
}

int sendevent(client_t *c, Atom proto){
	int n;
	Atom *protocols;
	int exists = 0;
	XEvent ev;


	if(XGetWMProtocols(dwm.dpy, c->win, &protocols, &n)){
		while(!exists && n--)
			exists = protocols[n] == proto;
		XFree(protocols);
	}

	if(exists){
		ev.type = ClientMessage;
		ev.xclient.window = c->win;
		ev.xclient.message_type = dwm.wmatom[WMProtocols];
		ev.xclient.format = 32;
		ev.xclient.data.l[0] = proto;
		ev.xclient.data.l[1] = CurrentTime;
		XSendEvent(dwm.dpy, c->win, False, NoEventMask, &ev);
	}

	return exists;
}

void updatewmhints(client_t *c){
	XWMHints *wmh;


	if((wmh = XGetWMHints(dwm.dpy, c->win))){
		if(c == dwm.mons->sel && wmh->flags & XUrgencyHint){
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dwm.dpy, c->win, wmh);
		}

		if(wmh->flags & InputHint)	c->hints.never_focus = !wmh->input;
		else						c->hints.never_focus = 0;

		XFree(wmh);
	}
}

void updatesizehints(client_t *c){
	long msize;
	XSizeHints size;


	if(!XGetWMNormalHints(dwm.dpy, c->win, &size, &msize)){
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	}

	if(size.flags & PBaseSize){
		c->hints.width_base = size.base_width;
		c->hints.height_base = size.base_height;
	}
	else if(size.flags & PMinSize){
		c->hints.width_base = size.min_width;
		c->hints.height_base = size.min_height;
	}
	else
		c->hints.width_base = c->hints.height_base = 0;

	if(size.flags & PResizeInc){
		c->hints.width_inc = size.width_inc;
		c->hints.height_inc = size.height_inc;
	}
	else
		c->hints.width_inc = c->hints.height_inc = 0;

	if(size.flags & PMaxSize){
		c->hints.width_max = size.max_width;
		c->hints.height_max = size.max_height;
	}
	else
		c->hints.width_max = c->hints.height_max = 0;

	if(size.flags & PMinSize){
		c->hints.width_min = size.min_width;
		c->hints.height_min = size.min_height;
	}
	else if(size.flags & PBaseSize){
		c->hints.width_min = size.base_width;
		c->hints.height_min = size.base_height;
	}
	else
		c->hints.width_min = c->hints.height_min = 0;

	if(size.flags & PAspect){
		c->hints.aspect_min = (float)size.min_aspect.y / size.min_aspect.x;
		c->hints.aspect_max = (float)size.max_aspect.x / size.max_aspect.y;
	}
	else
		c->hints.aspect_max = c->hints.aspect_min = 0.0;
}


/* local functions */
static void updateclientlist(){
	client_t *c;
	monitor_t *m;


	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList]);

	for(m=dwm.mons; m; m=m->next){
		for(c=m->clients; c; c=c->next)
			XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);
	}
}

static int applysizehints(client_t *c, int *x, int *y, int *w, int *h, int interact){
	monitor_t *m = c->mon;
	client_geom_t *geom = &c->geom;
	int baseismin;


	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);

	if(interact){
		if(*x > dwm.screen_width)
			*x = dwm.screen_width - WIDTH(c);

		if(*y > dwm.screen_height)
			*y = dwm.screen_height - HEIGHT(c);

		if(*x + *w + 2 * geom->border_width < 0)
			*x = 0;

		if(*y + *h + 2 * geom->border_width < 0)
			*y = 0;
	}
	else{
		if(*x >= m->x + m->width)
			*x = m->x + m->width - WIDTH(c);

		if(*y >= m->y + m->height)
			*y = m->y + m->height - HEIGHT(c);

		if(*x + *w + 2 * geom->border_width <= m->x)
			*x = m->x;

		if(*y + *h + 2 * geom->border_width <= m->y)
			*y = m->y;
	}
	if(*h < dwm.statusbar.height)
		*h = dwm.statusbar.height;

	if(*w < dwm.statusbar.height)
		*w = dwm.statusbar.height;

	/* see last two sentences in ICCCM 4.1.2.3 */
	baseismin = c->hints.width_base == c->hints.width_min && c->hints.height_base == c->hints.height_min;

	if(!baseismin){ /* temporarily remove base dimensions */
		*w -= c->hints.width_base;
		*h -= c->hints.height_base;
	}

	/* adjust for aspect limits */
	if(c->hints.aspect_min > 0 && c->hints.aspect_max > 0){
		if(c->hints.aspect_max < (float)*w / *h)		*w = *h * c->hints.aspect_max + 0.5;
		else if(c->hints.aspect_min < (float)*h / *w)	*h = *w * c->hints.aspect_min + 0.5;
	}

	if(baseismin){ /* increment calculation requires this */
		*w -= c->hints.width_base;
		*h -= c->hints.height_base;
	}

	/* adjust for increment value */
	if(c->hints.width_inc)
		*w -= *w % c->hints.width_inc;

	if(c->hints.height_inc)
		*h -= *h % c->hints.height_inc;

	/* restore base dimensions */
	*w = MAX(*w + c->hints.width_base, c->hints.width_min);
	*h = MAX(*h + c->hints.height_base, c->hints.height_min);

	if(c->hints.width_max)
		*w = MIN(*w, c->hints.width_max);

	if(c->hints.height_max)
		*h = MIN(*h, c->hints.height_max);

	return *x != geom->x || *y != geom->y || *w != geom->width || *h != geom->height;
}

static void detach(client_t *c){
	client_t **tc;


	for(tc=&c->mon->clients; *tc && *tc!=c; tc=&(*tc)->next);

	*tc = c->next;
}

static Atom getatomprop(client_t *c, Atom prop){
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;


	if(XGetWindowProperty(dwm.dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM, &da, &di, &dl, &dl, &p) == Success && p){
		atom = *(Atom*)p;
		XFree(p);
	}

	return atom;
}

static void grabbuttons(client_t *c, int focused){
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = {0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask};


		XUngrabButton(dwm.dpy, AnyButton, AnyModifier, c->win);

		if(!focused)
			XGrabButton(dwm.dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);

		for(i=0; i<nbuttons; i++){
			if(buttons[i].click == ClkClientWin){
				for(j=0; j<LENGTH(modifiers); j++)
					XGrabButton(dwm.dpy, buttons[i].button, buttons[i].mask | modifiers[j], c->win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
			}
		}
	}
}
