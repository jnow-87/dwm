#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <utils.h>


/* global functions */
void tile(monitor_t *m){
	unsigned int i, n, h, w, y, ty;
	client_t *c;


	for(n=0, c=nexttiled(m->clients); c; c=nexttiled(c->next), n++);

	if(n == 0)
		return;

	if(n > m->nmaster)	w = m->nmaster ? m->width * m->mfact : 0;
	else				w = m->width;

	for(i=y=ty=0, c=nexttiled(m->clients); c; c=nexttiled(c->next), i++){
		if(i < m->nmaster){
			h = (m->height - y) / (MIN(n, m->nmaster) - i);
			resize(c, m->x, m->y + y, w - (2 * c->bw), h - (2 * c->bw), 0);

			if(y + HEIGHT(c) < m->height)
				y += HEIGHT(c);
		}
		else{
			h = (m->height - ty) / (n - i);
			resize(c, m->x + w, m->y + ty, m->width - w - (2 * c->bw), h - (2 * c->bw), 0);

			if(ty + HEIGHT(c) < m->height)
				ty += HEIGHT(c);
		}
	}
}
