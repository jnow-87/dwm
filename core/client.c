#include <config/config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <core/buttons.h>
#include <core/client.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <xlib/atoms.h>
#include <xlib/window.h>
#include <utils/list.h>
#include <utils/log.h>
#include <utils/stack.h>
#include <utils/utils.h>


/* macros */
#define PREP_N_STORE(dim, geom, store) \
	if(dim != (geom)->dim) \
		(store)->dim = (geom)->dim; \
	\
	(geom)->dim = dim
	

/* global functions */
int clients_init(void){
	unsigned int n;
	window_t *childs;
	window_t dummy;
	win_attr_t attr;


	if(!XQueryTree(dwm.dpy, dwm.root, &dummy, &dummy, &childs, &n))
		return ERROR("querying clients\n");

	for(unsigned int i=0; i<n; i++){
		if(win_get_attr(childs[i], &attr) != 0 || attr.override_redirect)
			continue;

		if(attr.map_state == IsViewable || win_get_state(childs[i]) == IconicState)
			client_init(childs[i], &attr);
	}

	XFree(childs);

	return 0;
}

void clients_cleanup(void){
	while(dwm.stack)
		client_cleanup(dwm.stack, false);
}

void client_init(window_t win, win_attr_t *attr){
	monitor_t *m = dwm.mons;
	client_t *c,
			 *trans;
	win_geom_t *geom;


	c = calloc(1, sizeof(client_t));

	if(c == 0x0)
		EEXIT("allocating new client\n");

	/* init client */
	c->win = win;
	c->flags = 0;

	trans = client_from_win(win_get_transient(win));
	c->tags = (trans != 0x0) ? trans->tags : dwm.tag_mask;

	geom = &c->geom;
	*geom = attr->geom;

	geom->x = (m->width - (geom->width + 2 * CONFIG_BORDER_PIXEL)) / 2;
	geom->y = (m->height - (geom->height + 2 * CONFIG_BORDER_PIXEL)) / 2;
	geom->border_width = CONFIG_BORDER_PIXEL;

	c->geom_store = attr->geom;

	win_init(win, &c->geom, &c->hints);
	buttons_register(c);

	/* update client list */
	netatom_append(NET_CLIENT_LIST, dwm.root, (unsigned char*)&win);

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
	netatom_delete(NET_CLIENT_LIST, dwm.root);

	list_for_each(dwm.stack, c)
		netatom_append(NET_CLIENT_LIST, dwm.root, (unsigned char*)&c->win);
}

client_t *client_from_win(window_t win){
	client_t *c;


	if(win == None)
		return 0x0;

	list_for_each(dwm.stack, c){
		if(c->win == win)
			return c;
	}

	return 0x0;
}

void client_resize(client_t *c, int x, int y, int width, int height, int border_width){
	win_geom_t *geom = &c->geom;


	if(x == geom->x && y == geom->y && width == geom->width && height == geom->height && geom->border_width == border_width)
		return;

	PREP_N_STORE(x, geom, &c->geom_store);
	PREP_N_STORE(y, geom, &c->geom_store);
	PREP_N_STORE(width, geom, &c->geom_store);
	PREP_N_STORE(height, geom, &c->geom_store);
	PREP_N_STORE(border_width, geom, &c->geom_store);

	win_resize(c->win, geom, 0x0);
}

void client_flags_set(client_t *c, unsigned int mask){
	win_geom_t geom;
	monitor_t *m;


	if(c->flags == mask)
		return;

	if(mask & (WF_FULLSCREEN | WF_MAXED)){
		m = monitor_from_client(c);

		geom.x = m->x;
		geom.y = m->y;
		geom.width = m->width;
		geom.height = m->height;
		geom.border_width = 0;

		// Pre-backup client geometry even though client_resize() does it already.
		// This is required, since changing the border doesn't impact the location
		// of a client, which causes client_resize() to not update the stored
		// location either, which in turn restores the wrong location when disabling
		// fullscreen.
		c->geom_store = c->geom;
	}
	else
		geom = c->geom_store;

	win_set_flags(c->win, mask);
	client_resize(c, geom.x, geom.y, geom.width, geom.height, geom.border_width);

	c->flags = mask;
}
