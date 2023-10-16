#include <config/config.h>
#include <core/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/monitor.h>
#include <xlib/window.h>
#include <utils/list.h>
#include <utils/utils.h>


/* macros */
#define NMASTER	CONFIG_TILED_MASTER_WINDOWS


/* local functions */
static void arrange(void){
	unsigned int i, n, h, w, y, ty;
	monitor_t *m;
	client_t *c;
	win_geom_t *geom;


	list_for_each(dwm.mons, m){
		for(n=0, c=layout_next_tiled(dwm.stack, m); c; c=layout_next_tiled(c->next, m), n++);

		if(n == 0)
			continue;

		if(n > NMASTER)	w = NMASTER ? m->width * (CONFIG_TILED_MASTER_RATIO / 100.0) : 0;
		else			w = m->width;

		for(i=y=ty=0, c=layout_next_tiled(dwm.stack, m); c; c=layout_next_tiled(c->next, m), i++){
			geom = &c->geom;

			if(i < NMASTER){
				h = (m->height - y) / (MIN(n, NMASTER) - i);
				client_resize(c, m->x, m->y + y, w - (2 * geom->border_width), h - (2 * geom->border_width));
				h = geom->height + 2 * geom->border_width;

				if(y + h < m->height)
					y += h;
			}
			else{
				h = (m->height - ty) / (n - i);
				client_resize(c, m->x + w, m->y + ty, m->width - w - (2 * geom->border_width), h - (2 * geom->border_width));
				h = geom->height + 2 * geom->border_width;

				if(ty + h < m->height)
					ty += h;
			}
		}
	}
}

LAYOUT("tiled", "⚏", arrange);
