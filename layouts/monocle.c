#include <xlib/window.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <stdio.h>
#include <utils/list.h>


/* global functions */
void layout_monocle(void){
	unsigned int n = 0;
	monitor_t *m;
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c))
			n++;
	}

	for(c=nexttiled(dwm.stack); c; c=nexttiled(c->next)){
		m = monitor_from_client(c);

		client_resize_with_hints(c, m->x, m->y, m->width - 2 * c->geom.border_width, m->height - 2 * c->geom.border_width, false);
	}
}
