#ifndef CLIENT_H
#define CLIENT_H


#include <stdbool.h>
#include <core/client.h>
#include <xlib/window.h>


/* macros */
#define ISVISIBLE(c)	((c->tags & dwm.tag_mask))
#define WIDTH(c)		((c)->geom.width + 2 * (c)->geom.border_width)
#define HEIGHT(c)		((c)->geom.height + 2 * (c)->geom.border_width)


/* types */
typedef struct client_t{
	struct client_t *prev,
					*next;

	unsigned int tags;

	window_t win;
	win_geom_t geom,
			   geom_store;
	win_hints_t hints;
} client_t;


/* prototypes */
int clients_init(void);
void clients_cleanup(void);

void client_init(window_t win, win_attr_t *attr);
void client_cleanup(client_t *c, bool destroyed);

client_t *client_from_win(window_t win);

void client_resize(client_t *c, int x, int y, int width, int height);


#endif // CLIENT_H
