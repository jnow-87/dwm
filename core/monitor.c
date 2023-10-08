#include <xlib/window.h>
#include <config.h>
#include <config/config.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <core/monitor.h>
#include <xlib/xinerama.h>
#include <utils/math.h>
#include <utils/list.h>


/* global functions */
void monitor_discover(void){
	/* free existing monitors */
	while(dwm.mons){
		monitor_destroy(dwm.mons);
	}

	if(xinerama_discover() < 0)
		monitor_create(0, 0, dwm.screen_width, dwm.screen_height);
}

void monitor_cleanup(void){
	while(dwm.mons){
		monitor_destroy(dwm.mons);
	}
}

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
	win_geom_t *geom = &c->geom;
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
