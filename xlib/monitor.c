#include <client.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <layout.h>
#include <statusbar.h>
#include <utils.h>


/* global functions */
monitor_t *createmon(void){
	monitor_t *m;


	m = ecalloc(1, sizeof(monitor_t));
	m->tagset[0] = m->tagset[1] = 1;

	// TODO the following values can prbably be hard-coded
	m->mfact = CONFIG_LAYOUT_MASTER_RATIO / 100.0;
	m->nmaster = CONFIG_LAYOUT_MASTER_WINDOWS;
	m->showbar = CONFIG_STATUSBAR_SHOW;
	m->topbar = CONFIG_STATUSBAR_TOP;

	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % nlayouts];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);

	return m;
}

monitor_t *dirtomon(int dir){
	monitor_t *m = NULL;


	if(dir > 0){
		if(!(m = selmon->next))
			m = mons;
	}
	else if(selmon == mons){
		for(m=mons; m->next; m=m->next);
	}
	else
		for(m=mons; m->next!=selmon; m=m->next);

	return m;
}

monitor_t *recttomon(int x, int y, int w, int h){
	monitor_t *m, *r = selmon;
	int a, area = 0;


	for(m=mons; m; m=m->next){
		if((a = INTERSECT(x, y, w, h, m)) > area){
			area = a;
			r = m;
		}
	}

	return r;
}

monitor_t *wintomon(Window w){
	int x, y;
	client_t *c;
	monitor_t *m;


	if(w == root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);

	for(m=mons; m; m=m->next){
		if(w == m->barwin)
			return m;
	}

	if((c = wintoclient(w)))
		return c->mon;

	return selmon;
}

void cleanupmon(monitor_t *mon){
	monitor_t *m;


	if(mon != mons){
		for(m=mons; m && m->next!=mon; m=m->next);

		m->next = mon->next;
	}
	else
		mons = mons->next;

	XUnmapWindow(dpy, mon->barwin);
	XDestroyWindow(dpy, mon->barwin);
	free(mon);
}

void restack(monitor_t *m){
	client_t *c;
	XEvent ev;
	XWindowChanges wc;


	drawbar(m);

	if(!m->sel)
		return;

	if(m->sel->isfloating || !m->lt[m->sellt]->arrange)
		XRaiseWindow(dpy, m->sel->win);

	if(m->lt[m->sellt]->arrange){
		wc.stack_mode = Below;
		wc.sibling = m->barwin;

		for(c=m->stack; c; c=c->snext){
			if(!c->isfloating && ISVISIBLE(c)){
				XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
				wc.sibling = c->win;
			}
		}
	}

	XSync(dpy, False);

	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}
