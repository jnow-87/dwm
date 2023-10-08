#include <config/config.h>
#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <utils.h>


/* macros */
#define NMASTER	CONFIG_LAYOUT_MASTER_WINDOWS


/* global functions */
void tile(monitor_t *m){
	unsigned int i, n, h, w, y, ty;
	client_t *c;
	client_geom_t *geom;


	for(n=0, c=nexttiled(m->clients); c; c=nexttiled(c->next), n++);

	if(n == 0)
		return;

	if(n > NMASTER)	w = NMASTER ? m->width * (CONFIG_LAYOUT_MASTER_RATIO / 100.0) : 0;
	else			w = m->width;

	for(i=y=ty=0, c=nexttiled(m->clients); c; c=nexttiled(c->next), i++){
		geom = &c->geom;

		if(i < NMASTER){
			h = (m->height - y) / (MIN(n, NMASTER) - i);
			resize(c, m->x, m->y + y, w - (2 * geom->border_width), h - (2 * geom->border_width), 0);

			if(y + HEIGHT(c) < m->height)
				y += HEIGHT(c);
		}
		else{
			h = (m->height - ty) / (n - i);
			resize(c, m->x + w, m->y + ty, m->width - w - (2 * geom->border_width), h - (2 * geom->border_width), 0);

			if(ty + HEIGHT(c) < m->height)
				ty += HEIGHT(c);
		}
	}
}
