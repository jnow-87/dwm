#include <core/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/monitor.h>
#include <utils/list.h>


/* local functions */
static void arrange(void){
	int border;
	monitor_t *m;
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ONTAG(c) && c->fades == 0){
			border = c->geom.border_width;
			m = c->mon;

			client_resize(c, m->x, m->y, m->width - 2 * border, m->height - 2 * border, border);
		}
	}
}

LAYOUT("monocle", "❍", arrange);
