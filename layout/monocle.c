#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <stdio.h>


/* global functions */
void monocle(void){
	unsigned int n = 0;
	monitor_t *m;
	client_t *c;


	for(c=dwm.clients; c; c=c->next){
		if(ISVISIBLE(c))
			n++;
	}

	for(c=nexttiled(dwm.clients); c; c=nexttiled(c->next)){
		m = client_to_monitor(c);

		resize(c, m->x, m->y, m->width - 2 * c->geom.border_width, m->height - 2 * c->geom.border_width, 0);
	}
}
