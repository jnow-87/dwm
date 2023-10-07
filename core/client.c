#include <config/config.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <config.h>
#include <core/client.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/scheme.h>
#include <core/stack.h>
#include <xlib/input.h>
#include <utils/list.h>
#include <utils/stack.h>
#include <utils/math.h>


/* global functions */
void client_init(Window w, XWindowAttributes *wa){
	monitor_t *m = dwm.mons;
	client_t *c, *t = NULL;
	Window trans = None;
	win_geom_t *geom;
	XWindowChanges wc;


	c = calloc(1, sizeof(client_t));

	if(c == 0x0)
		dwm_die("unable to allocate new client\n");

	c->win = w;
	geom = &c->geom;

	/* init client struct and win_configure the xlib client accordingly */
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
	win_configure(c->win, &c->geom); /* propagates border_width, if size doesn't change */
	win_update_sizehints(c->win, &c->hints);
	win_update_wmhints(c->win, &c->hints, false);
	XSelectInput(dwm.dpy, w, FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	input_register_button_mappings(c->win, buttons, nbuttons, 0);

	XRaiseWindow(dwm.dpy, c->win);
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);

	XMoveResizeWindow(dwm.dpy, c->win, geom->x + 2 * dwm.screen_width, geom->y, geom->width, geom->height); /* some windows require this */
	win_set_state(c->win, NormalState);

	XMapWindow(dwm.dpy, c->win);

	stack_push(dwm.stack, c);
	client_focus(c, true);

	// TODO move layout_arrange out of the function to avoid calling it multiple times during startup
	layout_arrange();
}

void client_cleanup(client_t *c, bool destroyed){
	list_rm(dwm.stack, c);
	client_refocus();

	if(!destroyed)
		win_release(c->win, &c->geom_store);

	free(c);

	/* update client list */
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList]);

	list_for_each(dwm.stack, c)
		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);

	layout_arrange();
	statusbar_raise();
}

client_t *client_from_win(Window w){
	client_t *c;


	list_for_each(dwm.stack, c){
		if(c->win == w)
			return c;
	}

	return 0x0;
}

void client_resize(client_t *c, int x, int y, int width, int height){
	win_geom_t *geom = &c->geom;


	if(x == geom->x && y == geom->y && width == geom->width && height == geom->height)
		return;

	c->geom_store = *geom;

	geom->x = x;
	geom->y = y;
	geom->width = width;
	geom->height = height;

	win_resize(c->win, geom, &c->hints);
}
