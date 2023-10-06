#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <xlib/client.h>
#include <core/scheme.h>
#include <config.h>
#include <config/config.h>
#include <core/dwm.h>
#include <xlib/atoms.h>
#include <core/xevents.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <utils/math.h>
#include <utils/list.h>
#include <utils/stack.h>


/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)


/* local/static prototypes */
static int apply_sizehints(client_t *c, int *x, int *y, int *w, int *h, int interact);
static void grab_buttons(client_t *c, int focused);


/* global functions */
client_t *client_from_win(Window w){
	client_t *c;


	list_for_each(dwm.stack, c){
		if(c->win == w)
			return c;
	}

	return NULL;
}

void client_init(Window w, XWindowAttributes *wa){
	monitor_t *m = dwm.mons;
	client_t *c, *t = NULL;
	Window trans = None;
	client_geom_t *geom;
	XWindowChanges wc;


	c = calloc(1, sizeof(client_t));

	if(c == 0x0)
		die("unable to allocate new client\n");

	c->win = w;
	geom = &c->geom;

	/* init client struct and client_configure the xlib client accordingly */
	geom->x = wa->x;
	geom->y = wa->y;
	geom->width = wa->width;
	geom->height = wa->height;
	geom->border_width = wa->border_width;

	c->geom_store = *geom;
	c->tags = dwm.tag_mask;

	if(XGetTransientForHint(dwm.dpy, w, &trans) && (t = client_from_win(trans)))
		c->tags = t->tags;

	if(geom->x + WIDTH(c) > m->x + m->width)
		geom->x = m->x + m->width - WIDTH(c);

	if(geom->y + HEIGHT(c) > m->y + m->height)
		geom->y = m->y + m->height - HEIGHT(c);

	geom->x = MAX(geom->x, m->x);
	geom->y = MAX(geom->y, m->y);
	geom->border_width = CONFIG_BORDER_PIXEL;

	wc.border_width = geom->border_width;
	XConfigureWindow(dwm.dpy, w, CWBorderWidth, &wc);
	XSetWindowBorder(dwm.dpy, w, dwm.scheme[SchemeNorm][ColBorder].pixel);
	client_configure(c); /* propagates border_width, if size doesn't change */
	client_update_sizehints(c);
	client_update_wmhints(c);
	XSelectInput(dwm.dpy, w, FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	grab_buttons(c, 0);

	XRaiseWindow(dwm.dpy, c->win);
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);

	XMoveResizeWindow(dwm.dpy, c->win, geom->x + 2 * dwm.screen_width, geom->y, geom->width, geom->height); /* some windows require this */
	client_set_state(c, NormalState);

	XMapWindow(dwm.dpy, c->win);

	stack_push(dwm.stack, c);
	client_focus(c, true);

	// TODO move layout_arrange out of the function to avoid calling it multiple times during startup
	layout_arrange();
}

void client_cleanup(client_t *c, int destroyed){
	XWindowChanges wc;


	list_rm(dwm.stack, c);

	if(!destroyed){
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

	free(c);

	client_refocus();

	/* update client list */
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList]);

	list_for_each(dwm.stack, c)
		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);

	layout_arrange();
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

void client_configure(client_t *c){
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

client_t *client_cycle(int dir, cycle_state_t state){
	static client_t *cycle_origin = 0x0;
	client_t *c;


	switch(state){
	case CYCLE_START:
		cycle_origin = dwm.stack;

		// fall through
	case CYCLE_CONT:
		// find the next visible client, either starting at the currently focused one
		// or restart at the top of the client stack
		c = dwm.focused;
		c = (c != 0x0) ? ((dir > 0) ? c->next : c->prev) : dwm.stack;

		for(; c!=0x0; c=(dir > 0) ? c->next : c->prev){
			if(ISVISIBLE(c))
				return c;
		}

		if(dwm.focused == 0x0) // the entire stack has been checked and nothing has bee found
			return 0x0;

		// retry, this time from the top of the stack
		dwm.focused = 0x0;

		return client_cycle(dir, 0);

	case CYCLE_END:
		if(cycle_origin != 0x0){
			stack_raise(dwm.stack, cycle_origin);
			cycle_origin = 0x0;
		}

		if(dwm.focused != 0x0)
			stack_raise(dwm.stack, dwm.focused);

		return dwm.stack;
	}

	return 0x0;
}

void client_refocus(void){
	client_t *c;


	dwm.focused = 0x0;

	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c) && !c->hints.never_focus)
			break;
	}

	client_focus(c, true);
}

void client_focus(client_t *c, bool restack){
	if(c == dwm.focused)
		return;

	/* unfocus client */
	if(dwm.focused){
		grab_buttons(dwm.focused, 0);
		XSetWindowBorder(dwm.dpy, dwm.focused->win, dwm.scheme[SchemeNorm][ColBorder].pixel);
	}

	/* client_focus */
	dwm.focused = c;

	if(c != 0x0){
		if(restack)
			stack_raise(dwm.stack, c);

		grab_buttons(c, 1);

		XSetWindowBorder(dwm.dpy, c->win, dwm.scheme[SchemeSel][ColBorder].pixel);
		XSetInputFocus(dwm.dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(c->win), 1);
		XRaiseWindow(dwm.dpy, c->win);
		client_send_event(c, dwm.wmatom[WMTakeFocus]);
	}
	else{
		XSetInputFocus(dwm.dpy, dwm.root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	}
}

void client_showhide(void){
	// TODO move to dwm level
	client_t *c;
	client_geom_t *geom;


	list_for_each(dwm.stack, c){
		geom = &c->geom;

		// TODO
		// 	This function has been changed to use the client list instead of
		// 	the stack. Originally, hiding and showing work top-down and bottom-up
		// 	respectively. This might cause some issues with correct overlapping
		// 	of windows.
		if(ISVISIBLE(c)){
			XMoveWindow(dwm.dpy, c->win, geom->x, geom->y);
			client_resize_with_hints(c, geom->x, geom->y, geom->width, geom->height, 0);
		}
		else{
			XMoveWindow(dwm.dpy, c->win, WIDTH(c) * -2, geom->y);
		}
	}
}

void client_resize_with_hints(client_t *c, int x, int y, int w, int h, int interact){
	c->geom_store = c->geom;

	if(apply_sizehints(c, &x, &y, &w, &h, interact))
		client_resize(c, x, y, w, h);
}

void client_resize(client_t *c, int x, int y, int w, int h){
	client_geom_t *geom = &c->geom;
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

int client_send_event(client_t *c, Atom proto){
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
static int apply_sizehints(client_t *c, int *x, int *y, int *w, int *h, int interact){
	monitor_t *m = monitor_from_client(c);
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

static void grab_buttons(client_t *c, int focused){
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
