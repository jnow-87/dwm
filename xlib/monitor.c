#include <client.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <layout.h>
#include <statusbar.h>
#include <monitor.h>
#include <utils.h>
#include <list.h>


/* global functions */
monitor_t *monitor_create(int x, int y, int width, int height){
	monitor_t *m;


	m = malloc(sizeof(monitor_t));

	if(m == 0x0)
		return 0x0;

	m->x = x;
	m->y = y;
	m->width = width;
	m->height = height;

	list_add_tail(dwm.mons, m);

	return m;
}

void monitor_destroy(monitor_t *m){
	list_rm(dwm.mons, m);
	free(m);
}

monitor_t *monitor_from_client(client_t *c){
	int area = 0;
	monitor_t *r = dwm.mons;
	client_geom_t *geom = &c->geom;
	monitor_t *m;
	int a;


	list_for_each(dwm.mons, m){
		a = MAX(0, MIN(geom->x + geom->width, m->x + m->width) - MAX(geom->x, m->x))
		  * MAX(0, MIN(geom->y + geom->height, m->y + m->height) - MAX(geom->y, m->y));

		if(a > area){
			area = a;
			r = m;
		}
	}

	return r;
}
