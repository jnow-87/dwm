#include <config/config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <X11/Xlib.h>
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


/* global functions */
int clients_init(void){
	unsigned int n;
	window_t *childs;
	window_t dummy;
	win_attr_t attr;


	if(!XQueryTree(dwm.dpy, dwm.root, &dummy, &dummy, &childs, &n))
		return ERROR("querying clients\n");

	// TODO what is the transient check for
	for(unsigned int i=0; i<n; i++){
		if(win_get_attr(childs[i], &attr) != 0 || attr.override_redirect || win_get_transient(childs[i]) != None)
			continue;

		if(attr.map_state == IsViewable || win_get_state(childs[i]) == IconicState)
			client_init(childs[i], &attr);
	}

	/* now the transients */
	for(unsigned int i=0; i<n; i++){
		if(win_get_attr(childs[i], &attr) != 0)
			continue;

		if(win_get_transient(childs[i]) != None && (attr.map_state == IsViewable || win_get_state(childs[i]) == IconicState))
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

	trans = client_from_win(win_get_transient(win));
	c->tags = (trans != 0x0) ? trans->tags : dwm.tag_mask;

	geom = &c->geom;
	*geom = attr->geom;
	geom->x = MAX(geom->x, m->x);
	geom->y = MAX(geom->y, m->y);
	geom->border_width = CONFIG_BORDER_PIXEL;

	c->geom_store = attr->geom;

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
