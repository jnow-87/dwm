#include <core/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <xlib/window.h>
#include <utils/list.h>


/* default nop layout */
LAYOUT("nop", "☯", 0x0);


/* global functions */
client_t *layout_next_tiled(client_t *c, monitor_t *m){
	for(; c != 0x0; c=c->next){
		if(ONTAG(c) && monitor_from_client(c) == m)
			return c;
	}

	return 0x0;
}

void layout_arrange(void){
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ONTAG(c))	win_show(c->win, &c->geom);
		else				win_hide(c->win, &c->geom);
	}

	if(dwm.layout->arrange)
		dwm.layout->arrange();

	statusbar_raise();
}
