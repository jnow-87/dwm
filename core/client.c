#include <config/config.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <config.h>
#include <core/client.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/scheme.h>
#include <core/clientstack.h>
#include <xlib/input.h>
#include <xlib/atoms.h>
#include <utils/list.h>
#include <utils/stack.h>
#include <utils/math.h>


/* global functions */
void client_init(Window win, XWindowAttributes *wa){
	monitor_t *m = dwm.mons;
	client_t *c,
			 *trans;
	win_geom_t *geom;


	c = calloc(1, sizeof(client_t));

	if(c == 0x0)
		dwm_die("unable to allocate new client\n");

	/* init client */
	c->win = win;

	trans = client_from_win(win_get_transient(win));
	c->tags = (trans != 0x0) ? trans->tags : dwm.tag_mask;

	geom = &c->geom;
	geom->x = MAX(wa->x, m->x);
	geom->y = MAX(wa->y, m->y);
	geom->width = wa->width;
	geom->height = wa->height;
	geom->border_width = CONFIG_BORDER_PIXEL;

	c->geom_store = *geom;
	c->geom_store.border_width = wa->border_width;

	win_init(win, &c->geom, &c->hints);

	/* update client list */
	atoms_netatom_append(NetClientList, (unsigned char*)&win);

	/* update clientstack */
	stack_push(dwm.stack, c);
	clientstack_focus(c, true);
}

void client_cleanup(client_t *c, bool destroyed){
	/* update clientstack */
	list_rm(dwm.stack, c);
	clientstack_refocus();

	/* destroy */
	if(!destroyed)
		win_release(c->win, &c->geom_store);

	free(c);

	/* update client list */
	atoms_netatom_delete(NetClientList);

	list_for_each(dwm.stack, c)
		atoms_netatom_append(NetClientList, (unsigned char*)&c->win);
}

client_t *client_from_win(Window win){
	client_t *c;


	if(win == None)
		return 0x0;

	list_for_each(dwm.stack, c){
		if(c->win == win)
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
