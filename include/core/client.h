#ifndef CLIENT_H
#define CLIENT_H


#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xlib.h>
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

	Window win;
	win_geom_t geom,
			   geom_store;
	win_hints_t hints;
} client_t;


/* prototypes */
void client_init(Window win, XWindowAttributes *wa);
void client_cleanup(client_t *c, bool destroyed);

client_t *client_from_win(Window win);

void client_resize(client_t *c, int x, int y, int width, int height);


#endif // CLIENT_H
