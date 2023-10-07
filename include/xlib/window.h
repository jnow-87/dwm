#ifndef CLIENT_H
#define CLIENT_H


#include <stdbool.h>
#include <X11/Xlib.h>

struct client_t;

/* types */
typedef struct{
	int x,
		y,
		width,
		height;

	int border_width;
} win_geom_t;


/* prototypes */
void client_show(struct client_t *c);
void client_hide(struct client_t *c);

void client_kill(Window win);
void client_release(struct client_t *c);

void client_configure(struct client_t *c);

void client_resize_with_hints(struct client_t *c, int x, int y, int w, int h, bool interact);
void client_resize(struct client_t *c, int x, int y, int w, int h);
void client_set_state(struct client_t *c, long state);

bool client_send_event(struct client_t *c, Atom proto);

void client_update_wmhints(struct client_t *c);
void client_update_sizehints(struct client_t *c);


#endif // CLIENT_H
