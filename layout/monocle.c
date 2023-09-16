#include <stdio.h>
#include <dwm.h>
#include <client.h>
#include <layout.h>


/* global functions */
void monocle(monitor_t *m){
	unsigned int n = 0;
	client_t *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;

	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx, m->wy, m->ww - 2 * c->bw, m->wh - 2 * c->bw, 0);
}
