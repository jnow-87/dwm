#include <config/config.h>
#include <xlib/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <utils/math.h>


/* macros */
#define NMASTER	CONFIG_LAYOUT_MASTER_WINDOWS


/* global functions */
void layout_tile(void){
	// TODO
	// 	has to be re-implemented to layout_tile the clients on each monitor separately
	// 	since clients no longer have the monitor as struct member, the list of
	// 	clients for a particular monitor has to be computed first
	unsigned int i, n, h, w, y, ty;
	monitor_t *m = dwm.mons;
	client_t *c;
	client_geom_t *geom;


	for(n=0, c=nexttiled(dwm.stack); c; c=nexttiled(c->next), n++);

	if(n == 0)
		return;

	if(n > NMASTER)	w = NMASTER ? m->width * (CONFIG_LAYOUT_MASTER_RATIO / 100.0) : 0;
	else			w = m->width;

	for(i=y=ty=0, c=nexttiled(dwm.stack); c; c=nexttiled(c->next), i++){
		geom = &c->geom;

		if(i < NMASTER){
			h = (m->height - y) / (MIN(n, NMASTER) - i);
			client_resize_with_hints(c, m->x, m->y + y, w - (2 * geom->border_width), h - (2 * geom->border_width), 0);

			if(y + HEIGHT(c) < m->height)
				y += HEIGHT(c);
		}
		else{
			h = (m->height - ty) / (n - i);
			client_resize_with_hints(c, m->x + w, m->y + ty, m->width - w - (2 * geom->border_width), h - (2 * geom->border_width), 0);

			if(ty + HEIGHT(c) < m->height)
				ty += HEIGHT(c);
		}
	}
}
