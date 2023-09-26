#include <client.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <layout.h>
#include <statusbar.h>
#include <utils.h>


/* macros */
#define INTERSECT(x,y,w,h,m) \
	  (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
	* MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))


/* global functions */
monitor_t *createmon(void){
	monitor_t *m;


	m = ecalloc(1, sizeof(monitor_t));
	m->tagset[0] = m->tagset[1] = 1;

	// TODO the following values can prbably be hard-coded
	m->mfact = CONFIG_LAYOUT_MASTER_RATIO / 100.0;
	m->nmaster = CONFIG_LAYOUT_MASTER_WINDOWS;

	m->lt[0] = &layouts[0];
	m->lt[1] = &layouts[1 % nlayouts];
	strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);

	return m;
}

monitor_t *recttomon(int x, int y, int w, int h){
	monitor_t *m, *r = dwm.mons;
	int a, area = 0;


	for(m=dwm.mons; m; m=m->next){
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


	if(w == dwm.root && getrootptr(&x, &y))
		return recttomon(x, y, 1, 1);

	if(w == dwm.statusbar.win)
		return dwm.mons;

	if((c = wintoclient(w)))
		return c->mon;

	return dwm.mons;
}

void cleanupmon(monitor_t *mon){
	monitor_t *m;


	if(mon != dwm.mons){
		for(m=dwm.mons; m && m->next!=mon; m=m->next);

		m->next = mon->next;
	}
	else
		dwm.mons = dwm.mons->next;

	statusbar_destroy();
	free(mon);
}

void restack(monitor_t *m){
	XEvent ev;


	statusbar_draw();

	if(!m->sel)
		return;

	XRaiseWindow(dwm.dpy, m->sel->win);
	XSync(dwm.dpy, False);

	while(XCheckMaskEvent(dwm.dpy, EnterWindowMask, &ev));
}
