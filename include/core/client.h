#ifndef CLIENT_H
#define CLIENT_H


#include <stdbool.h>
#include <core/client.h>
#include <core/monitor.h>
#include <xlib/window.h>


/* macros */
#define ONTAG(c)	((c)->tags & dwm.tag_mask)


/* types */
typedef struct client_t{
	struct client_t *prev,
					*next;

	unsigned int tags,
				 fades;

	window_t win;
	monitor_t *mon;
	win_geom_t geom,
			   geom_store;
	win_hints_t hints;
	win_flags_t flags;
} client_t;


/* prototypes */
int clients_init(void);
void clients_cleanup(void);

void client_init(window_t win, win_attr_t *attr);
void client_cleanup(client_t *c, bool destroyed);

client_t *client_from_win(window_t win);

void client_resize(client_t *c, int x, int y, int width, int height, int border_width);
void client_flags_set(client_t *c, unsigned int mask);
void client_update_desktop(client_t *c);


#endif // CLIENT_H
