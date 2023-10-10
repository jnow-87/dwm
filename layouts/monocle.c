#include <core/client.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/monitor.h>
#include <utils/list.h>


/* local functions */
static void layout_monocle(void){
	unsigned int n = 0;
	monitor_t *m;
	client_t *c;


	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c))
			n++;
	}

	for(c=layout_next_tiled(dwm.stack); c; c=layout_next_tiled(c->next)){
		m = monitor_from_client(c);

		client_resize(c, m->x, m->y, m->width - 2 * c->geom.border_width, m->height - 2 * c->geom.border_width);
	}
}

LAYOUT("❍", layout_monocle);
