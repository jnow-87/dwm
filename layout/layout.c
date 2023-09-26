#include <client.h>
#include <dwm.h>
#include <layout.h>
#include <monitor.h>


/* local/static prototypes */
static void arrangemon(monitor_t *m);


/* global functions */
client_t *nexttiled(client_t *c){
	for(; c && !ISVISIBLE(c); c=c->next);

	return c;
}

void arrange(monitor_t *m){
	if(m != 0x0){
		showhide(m->stack);
		arrangemon(m);
		restack(m);
	}
	else{
		for(m=dwm.mons; m; m=m->next)
			showhide(m->stack);

		for(m=dwm.mons; m; m=m->next)
			arrangemon(m);
	}
}


/* local functions */
static void arrangemon(monitor_t *m){
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);

	if(m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
}
