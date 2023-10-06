#include <xlib/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <xlib/monitor.h>



/* global functions */
client_t *nexttiled(client_t *c){
	for(; c && !ISVISIBLE(c); c=c->next);

	return c;
}

void layout_arrange(void){
	client_showhide();

	if(dwm.layout->layout_arrange)
		dwm.layout->layout_arrange();
}
