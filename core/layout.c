#include <core/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <xlib/window.h>
#include <utils/list.h>


/* default nop layout */
LAYOUT("☯", 0x0);


/* global functions */
client_t *layout_next_tiled(client_t *c){
	for(; c && !ISVISIBLE(c); c=c->next);

	return c;
}

void layout_arrange(void){
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c))	win_show(c->win, &c->geom);
		else				win_hide(c->win, &c->geom);
	}

	if(dwm.layout->arrange)
		dwm.layout->arrange();
}
