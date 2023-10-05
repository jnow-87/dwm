#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <stdio.h>
#include <list.h>


/* global functions */
void monocle(void){
	unsigned int n = 0;
	monitor_t *m;
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c))
			n++;
	}

	for(c=nexttiled(dwm.stack); c; c=nexttiled(c->next)){
		m = monitor_from_client(c);

		resize(c, m->x, m->y, m->width - 2 * c->geom.border_width, m->height - 2 * c->geom.border_width, 0);
	}
}
