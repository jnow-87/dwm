#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <xlib/window.h>
#include <core/scheme.h>
#include <config.h>
#include <config/config.h>
#include <core/dwm.h>
#include <xlib/atoms.h>
#include <xlib/input.h>
#include <core/xevents.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <utils/math.h>
#include <utils/list.h>
#include <utils/stack.h>


/* local/static prototypes */
static int apply_sizehints(client_t *c, int *x, int *y, int *w, int *h, bool interact);
static int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee);


/* global functions */
void client_show(client_t *c){
	win_geom_t *geom = &c->geom;


	XMoveWindow(dwm.dpy, c->win, geom->x, geom->y);
	client_resize_with_hints(c, geom->x, geom->y, geom->width, geom->height, false);
}

void client_hide(client_t *c){
	XMoveWindow(dwm.dpy, c->win, WIDTH(c) * -2, c->geom.y);
}

void client_kill(Window win){
	XGrabServer(dwm.dpy);
	XSetErrorHandler(dummy_xerror_hdlr);
	XSetCloseDownMode(dwm.dpy, DestroyAll);

	XKillClient(dwm.dpy, dwm.focused->win);
	XSync(dwm.dpy, False);

	XSetErrorHandler(xerror_hdlr);
	XUngrabServer(dwm.dpy);
}

void client_release(client_t *c){
	XWindowChanges wc;


	wc.border_width = c->geom_store.border_width;

	XGrabServer(dwm.dpy); /* avoid race conditions */
	XSetErrorHandler(dummy_xerror_hdlr);

	XSelectInput(dwm.dpy, c->win, NoEventMask);
	XConfigureWindow(dwm.dpy, c->win, CWBorderWidth, &wc); /* restore border */
	XUngrabButton(dwm.dpy, AnyButton, AnyModifier, c->win);
	client_set_state(c, WithdrawnState);
	XSync(dwm.dpy, False);

	XSetErrorHandler(xerror_hdlr);
	XUngrabServer(dwm.dpy);
}

void client_configure(client_t *c){
	win_geom_t *geom = &c->geom;
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

void client_resize_with_hints(client_t *c, int x, int y, int w, int h, bool interact){
	c->geom_store = c->geom;

	if(apply_sizehints(c, &x, &y, &w, &h, interact))
		client_resize(c, x, y, w, h);
}

void client_resize(client_t *c, int x, int y, int w, int h){
	win_geom_t *geom = &c->geom;
	XWindowChanges wc;


	c->geom_store = *geom;
	geom->x = wc.x = x;
	geom->y = wc.y = y;
	geom->width = wc.width = w;
	geom->height = wc.height = h;

	wc.border_width = geom->border_width;

	XConfigureWindow(dwm.dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	client_configure(c);
	XSync(dwm.dpy, False);
}

void client_set_state(client_t *c, long state){
	long data[] = {state, None};


	XChangeProperty(dwm.dpy, c->win, dwm.wmatom[WMState], dwm.wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

bool client_send_event(client_t *c, Atom proto){
	bool exists = false;
	int n;
	Atom *protocols;
	XEvent ev;


	if(XGetWMProtocols(dwm.dpy, c->win, &protocols, &n)){
		while(!exists && n--)
			exists = (protocols[n] == proto);

		XFree(protocols);
	}

	if(!exists)
		return false;

	ev.type = ClientMessage;
	ev.xclient.window = c->win;
	ev.xclient.message_type = dwm.wmatom[WMProtocols];
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = proto;
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dwm.dpy, c->win, False, NoEventMask, &ev);

	return true;
}

void client_update_wmhints(client_t *c){
	XWMHints *wmh;


	if((wmh = XGetWMHints(dwm.dpy, c->win))){
		if(c == dwm.focused && wmh->flags & XUrgencyHint){
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dwm.dpy, c->win, wmh);
		}

		if(wmh->flags & InputHint)	c->hints.never_focus = !wmh->input;
		else						c->hints.never_focus = 0;

		XFree(wmh);
	}
}

void client_update_sizehints(client_t *c){
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
static int apply_sizehints(client_t *c, int *x, int *y, int *w, int *h, bool interact){
	monitor_t *m = monitor_from_client(c);
	win_geom_t *geom = &c->geom;
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

static int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee){
	return 0;
}
