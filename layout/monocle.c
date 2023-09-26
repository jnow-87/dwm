#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <stdio.h>


/* global functions */
void monocle(monitor_t *m){
	unsigned int n = 0;
	client_t *c;


	for(c=m->clients; c; c=c->next){
		if(ISVISIBLE(c))
			n++;
	}

	if(n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);

	for(c=nexttiled(m->clients); c; c=nexttiled(c->next))
		resize(c, m->x, m->y, m->width - 2 * c->bw, m->height - 2 * c->bw, 0);
}
