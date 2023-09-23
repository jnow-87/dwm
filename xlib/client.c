#include <X11/Xatom.h>
#include <client.h>
#include <colors.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <events.h>
#include <layout.h>
#include <statusbar.h>
#include <utils.h>


/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)


/* local/static prototypes */
static void updateclientlist();
static void applyrules(client_t *c);
static int applysizehints(client_t *c, int *x, int *y, int *w, int *h, int interact);
static void detach(client_t *c);
static Atom getatomprop(client_t *c, Atom prop);
static void grabbuttons(client_t *c, int focused);
static void updatesizehints(client_t *c);


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
	XWindowChanges wc;


	c = ecalloc(1, sizeof(client_t));
	c->win = w;

	/* geometry */
	c->x = c->oldx = wa->x;
	c->y = c->oldy = wa->y;
	c->w = c->oldw = wa->width;
	c->h = c->oldh = wa->height;
	c->oldbw = wa->border_width;

	updatetitle(c);

	if(XGetTransientForHint(dwm.dpy, w, &trans) && (t = wintoclient(trans))){
		c->mon = t->mon;
		c->tags = t->tags;
	}
	else{
		c->mon = dwm.selmon;
		applyrules(c);
	}

	if(c->x + WIDTH(c) > c->mon->wx + c->mon->ww)
		c->x = c->mon->wx + c->mon->ww - WIDTH(c);

	if(c->y + HEIGHT(c) > c->mon->wy + c->mon->wh)
		c->y = c->mon->wy + c->mon->wh - HEIGHT(c);

	c->x = MAX(c->x, c->mon->wx);
	c->y = MAX(c->y, c->mon->wy);
	c->bw = CONFIG_BORDER_PIXEL;

	wc.border_width = c->bw;
	XConfigureWindow(dwm.dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dwm.dpy, w, dwm.scheme[SchemeNorm][ColBorder].pixel);
	configure(c); /* propagates border_width, if size doesn't change */
	updatewindowtype(c);
	updatesizehints(c);
	updatewmhints(c);
	XSelectInput(dwm.dpy, w, FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grabbuttons(c, 0);

	if(!c->isfloating)
		c->isfloating = c->oldstate = trans != None || c->isfixed;

	if(c->isfloating)
		XRaiseWindow(dwm.dpy, c->win);

	attach(c);
	attachstack(c);
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);
	XMoveResizeWindow(dwm.dpy, c->win, c->x + 2 * dwm.screen_width, c->y, c->w, c->h); /* some windows require this */
	setclientstate(c, NormalState);

	if(c->mon == dwm.selmon)
		unfocus(dwm.selmon->sel, 0);

	c->mon->sel = c;
	arrange(c->mon);
	XMapWindow(dwm.dpy, c->win);
	focus(NULL);
}

void unmanage(client_t *c, int destroyed){
	monitor_t *m = c->mon;
	XWindowChanges wc;


	detach(c);
	detachstack(c);

	if(!destroyed){
		wc.border_width = c->oldbw;
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
	XKillClient(dwm.dpy, dwm.selmon->sel->win);
	XSync(dwm.dpy, False);
	XSetErrorHandler(xerror_hdlr);
	XUngrabServer(dwm.dpy);
}

void configure(client_t *c){
	XConfigureEvent ce;


	ce.type = ConfigureNotify;
	ce.display = dwm.dpy;
	ce.event = c->win;
	ce.window = c->win;
	ce.x = c->x;
	ce.y = c->y;
	ce.width = c->w;
	ce.height = c->h;
	ce.border_width = c->bw;
	ce.above = None;
	ce.override_redirect = False;
	XSendEvent(dwm.dpy, c->win, False, StructureNotifyMask, (XEvent*)&ce);
}

void attach(client_t *c){
	c->next = c->mon->clients;
	c->mon->clients = c;
}

void attachstack(client_t *c){
	c->snext = c->mon->stack;
	c->mon->stack = c;
}

void detachstack(client_t *c){
	client_t **tc, *t;


	for(tc=&c->mon->stack; *tc && *tc!=c; tc=&(*tc)->snext);

	*tc = c->snext;

	if(c == c->mon->sel){
		for(t=c->mon->stack; t && !ISVISIBLE(t); t=t->snext);

		c->mon->sel = t;
	}
}

void pop(client_t *c){
	detach(c);
	attach(c);
	focus(c);
	arrange(c->mon);
}

void focus(client_t *c){
	if(!c || !ISVISIBLE(c))
		for(c=dwm.selmon->stack; c && !ISVISIBLE(c); c=c->snext);

	if(dwm.selmon->sel && dwm.selmon->sel != c)
		unfocus(dwm.selmon->sel, 0);

	if(c){
		if(c->mon != dwm.selmon)
			dwm.selmon = c->mon;

		if(c->isurgent)
			seturgent(c, 0);

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

	dwm.selmon->sel = c;
	drawbars();
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
	if(!c)
		return;

	if(ISVISIBLE(c)){
		/* show clients top down */
		XMoveWindow(dwm.dpy, c->win, c->x, c->y);

		if((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && !c->isfullscreen)
			resize(c, c->x, c->y, c->w, c->h, 0);

		showhide(c->snext);
	}
	else{
		/* hide clients bottom up */
		showhide(c->snext);
		XMoveWindow(dwm.dpy, c->win, WIDTH(c) * -2, c->y);
	}
}

void sendmon(client_t *c, monitor_t *m){
	if(c->mon == m)
		return;

	unfocus(c, 1);
	detach(c);
	detachstack(c);

	c->mon = m;
	c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */

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
	XWindowChanges wc;


	c->oldx = c->x;
	c->x = wc.x = x;
	c->oldy = c->y;
	c->y = wc.y = y;
	c->oldw = c->w;
	c->w = wc.width = w;
	c->oldh = c->h;
	c->h = wc.height = h;
	wc.border_width = c->bw;

	XConfigureWindow(dwm.dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	configure(c);
	XSync(dwm.dpy, False);
}

void setclientstate(client_t *c, long state){
	long data[] = {state, None};


	XChangeProperty(dwm.dpy, c->win, dwm.wmatom[WMState], dwm.wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

void setfocus(client_t *c){
	if(!c->neverfocus){
		XSetInputFocus(dwm.dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(c->win), 1);
	}

	sendevent(c, dwm.wmatom[WMTakeFocus]);
}

void setfullscreen(client_t *c, int fullscreen){
	if(fullscreen && !c->isfullscreen){
		XChangeProperty(dwm.dpy, c->win, dwm.netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)&dwm.netatom[NetWMFullscreen], 1);
		c->isfullscreen = 1;
		c->oldstate = c->isfloating;
		c->oldbw = c->bw;
		c->bw = 0;
		c->isfloating = 1;
		resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
		XRaiseWindow(dwm.dpy, c->win);
	}
	else if(!fullscreen && c->isfullscreen){
		XChangeProperty(dwm.dpy, c->win, dwm.netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
		c->isfullscreen = 0;
		c->isfloating = c->oldstate;
		c->bw = c->oldbw;
		c->x = c->oldx;
		c->y = c->oldy;
		c->w = c->oldw;
		c->h = c->oldh;
		resizeclient(c, c->x, c->y, c->w, c->h);
		arrange(c->mon);
	}
}

void seturgent(client_t *c, int urg){
	XWMHints *wmh;


	c->isurgent = urg;

	if(!(wmh = XGetWMHints(dwm.dpy, c->win)))
		return;

	wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
	XSetWMHints(dwm.dpy, c->win, wmh);
	XFree(wmh);
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

void updatewindowtype(client_t *c){
	Atom state = getatomprop(c, dwm.netatom[NetWMState]);
	Atom wtype = getatomprop(c, dwm.netatom[NetWMWindowType]);


	if(state == dwm.netatom[NetWMFullscreen])
		setfullscreen(c, 1);

	if(wtype == dwm.netatom[NetWMWindowTypeDialog])
		c->isfloating = 1;
}

void updatewmhints(client_t *c){
	XWMHints *wmh;


	if((wmh = XGetWMHints(dwm.dpy, c->win))){
		if(c == dwm.selmon->sel && wmh->flags & XUrgencyHint){
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dwm.dpy, c->win, wmh);
		}
		else
			c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;

		if(wmh->flags & InputHint)	c->neverfocus = !wmh->input;
		else						c->neverfocus = 0;

		XFree(wmh);
	}
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

static void applyrules(client_t *c){
	char const *class, *instance;
	unsigned int i;
	rule_t const *r;
	monitor_t *m;
	XClassHint ch = {NULL, NULL};


	/* rule matching */
	c->isfloating = 0;
	c->tags = 0;
	XGetClassHint(dwm.dpy, c->win, &ch);
	class = ch.res_class ? ch.res_class : 0x0;
	instance = ch.res_name ? ch.res_name : 0x0;

	for(i=0; i<nrules; i++){
		r = &rules[i];

		if((!r->title || strstr(c->name, r->title)) && (!r->class || (class && strstr(class, r->class))) && (!r->instance || (instance && strstr(instance, r->instance)))){
			c->isfloating = r->isfloating;
			c->tags |= r->tags;

			for(m=dwm.mons; m && m->num!=r->monitor; m=m->next);

			if(m)
				c->mon = m;
		}
	}

	if(ch.res_class)
		XFree(ch.res_class);

	if(ch.res_name)
		XFree(ch.res_name);

	c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

static int applysizehints(client_t *c, int *x, int *y, int *w, int *h, int interact){
	int baseismin;
	monitor_t *m = c->mon;


	/* set minimum possible */
	*w = MAX(1, *w);
	*h = MAX(1, *h);

	if(interact){
		if(*x > dwm.screen_width)
			*x = dwm.screen_width - WIDTH(c);

		if(*y > dwm.screen_height)
			*y = dwm.screen_height - HEIGHT(c);

		if(*x + *w + 2 * c->bw < 0)
			*x = 0;

		if(*y + *h + 2 * c->bw < 0)
			*y = 0;
	}
	else{
		if(*x >= m->wx + m->ww)
			*x = m->wx + m->ww - WIDTH(c);

		if(*y >= m->wy + m->wh)
			*y = m->wy + m->wh - HEIGHT(c);

		if(*x + *w + 2 * c->bw <= m->wx)
			*x = m->wx;

		if(*y + *h + 2 * c->bw <= m->wy)
			*y = m->wy;
	}
	if(*h < dwm.statusbar_height)
		*h = dwm.statusbar_height;

	if(*w < dwm.statusbar_height)
		*w = dwm.statusbar_height;

	if(CONFIG_LAYOUT_RESPECT_SIZE_HINTS || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange){
		if(!c->hintsvalid)
			updatesizehints(c);

		/* see last two sentences in ICCCM 4.1.2.3 */
		baseismin = c->basew == c->minw && c->baseh == c->minh;

		if(!baseismin){ /* temporarily remove base dimensions */
			*w -= c->basew;
			*h -= c->baseh;
		}

		/* adjust for aspect limits */
		if(c->mina > 0 && c->maxa > 0){
			if(c->maxa < (float)*w / *h)		*w = *h * c->maxa + 0.5;
			else if(c->mina < (float)*h / *w)	*h = *w * c->mina + 0.5;
		}

		if(baseismin){ /* increment calculation requires this */
			*w -= c->basew;
			*h -= c->baseh;
		}

		/* adjust for increment value */
		if(c->incw)
			*w -= *w % c->incw;

		if(c->inch)
			*h -= *h % c->inch;

		/* restore base dimensions */
		*w = MAX(*w + c->basew, c->minw);
		*h = MAX(*h + c->baseh, c->minh);

		if(c->maxw)
			*w = MIN(*w, c->maxw);

		if(c->maxh)
			*h = MIN(*h, c->maxh);
	}

	return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
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
		for(i=0; i<nbuttons; i++)
			if(buttons[i].click == ClkClientWin)
				for(j=0; j<LENGTH(modifiers); j++)
					XGrabButton(dwm.dpy, buttons[i].button, buttons[i].mask | modifiers[j], c->win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
	}
}

static void updatesizehints(client_t *c){
	long msize;
	XSizeHints size;


	if(!XGetWMNormalHints(dwm.dpy, c->win, &size, &msize)){
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	}

	if(size.flags & PBaseSize){
		c->basew = size.base_width;
		c->baseh = size.base_height;
	}
	else if(size.flags & PMinSize){
		c->basew = size.min_width;
		c->baseh = size.min_height;
	}
	else
		c->basew = c->baseh = 0;

	if(size.flags & PResizeInc){
		c->incw = size.width_inc;
		c->inch = size.height_inc;
	}
	else
		c->incw = c->inch = 0;

	if(size.flags & PMaxSize){
		c->maxw = size.max_width;
		c->maxh = size.max_height;
	}
	else
		c->maxw = c->maxh = 0;

	if(size.flags & PMinSize){
		c->minw = size.min_width;
		c->minh = size.min_height;
	}
	else if(size.flags & PBaseSize){
		c->minw = size.base_width;
		c->minh = size.base_height;
	}
	else
		c->minw = c->minh = 0;

	if(size.flags & PAspect){
		c->mina = (float)size.min_aspect.y / size.min_aspect.x;
		c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
	}
	else
		c->maxa = c->mina = 0.0;

	c->isfixed = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
	c->hintsvalid = 1;
}
