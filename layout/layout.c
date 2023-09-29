#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <monitor.h>



/* global functions */
client_t *nexttiled(client_t *c){
	for(; c && !ISVISIBLE(c); c=c->next);

	return c;
}

void arrange(void){
	showhide(dwm.stack);

	if(dwm.layout->arrange)
		dwm.layout->arrange();
}
