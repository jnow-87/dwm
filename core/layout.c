#include <xlib/window.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/monitor.h>
#include <utils/list.h>


/* global functions */
client_t *nexttiled(client_t *c){
	for(; c && !ISVISIBLE(c); c=c->next);

	return c;
}

void layout_arrange(void){
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c))	win_show(c->win, &c->geom);
		else				win_hide(c->win, &c->geom);
	}

	if(dwm.layout->layout_arrange)
		dwm.layout->layout_arrange();
}
