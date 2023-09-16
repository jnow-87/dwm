#include <dwm.h>
#include <layout.h>


/* global functions */
client_t *nexttiled(client_t *c){
	for (; c && (c->isfloating || !ISVISIBLE(c)); c = c->next);
	return c;
}

void arrange(monitor_t *m){
	if (m)
		showhide(m->stack);
	else for (m = mons; m; m = m->next)
		showhide(m->stack);
	if (m) {
		arrangemon(m);
		restack(m);
	} else for (m = mons; m; m = m->next)
		arrangemon(m);
}

void arrangemon(monitor_t *m){
	strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
	if (m->lt[m->sellt]->arrange)
		m->lt[m->sellt]->arrange(m);
}


