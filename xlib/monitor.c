#include <client.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <layout.h>
#include <statusbar.h>
#include <monitor.h>
#include <utils.h>


/* global functions */
monitor_t *createmon(void){
	monitor_t *m;


	m = ecalloc(1, sizeof(monitor_t));

	return m;
}

monitor_t *client_to_monitor(client_t *c){
	int area = 0;
	monitor_t *r = dwm.mons;
	client_geom_t *geom = &c->geom;
	int a;


	for(monitor_t *m=dwm.mons; m; m=m->next){
		a = MAX(0, MIN(geom->x + geom->width, m->x + m->width) - MAX(geom->x, m->x))
		  * MAX(0, MIN(geom->y + geom->height, m->y + m->height) - MAX(geom->y, m->y));

		if(a > area){
			area = a;
			r = m;
		}
	}

	return r;
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

void restack(void){
	XEvent ev;


	statusbar_draw();

	if(!dwm.focused)
		return;

	XRaiseWindow(dwm.dpy, dwm.focused->win);
	XSync(dwm.dpy, False);

	while(XCheckMaskEvent(dwm.dpy, EnterWindowMask, &ev));
}
