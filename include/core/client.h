#ifndef CORE_CLIENT_H
#define CORE_CLIENT_H


#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <xlib/window.h>


/* macros */
#define ISVISIBLE(c)	((c->tags & dwm.tag_mask))
#define WIDTH(c)		((c)->geom.width + 2 * (c)->geom.border_width)
#define HEIGHT(c)		((c)->geom.height + 2 * (c)->geom.border_width)


/* types */
typedef struct{
	// size hints
	float aspect_min,
		  aspect_max;

	int width_base,
		width_min,
		width_max,
		width_inc,
		height_base,
		height_min,
		height_max,
		height_inc;

	// window manager hints
	int never_focus;
} client_hints_t;

typedef struct client_t{
	struct client_t *prev,
					*next;

	unsigned int tags;

	Window win;
	win_geom_t geom,
				  geom_store;
	client_hints_t hints;
} client_t;


/* prototypes */
void client_init(Window w, XWindowAttributes *wa);
void client_cleanup(client_t *c, bool destroyed);

client_t *client_from_win(Window w);


#endif // CORE_CLIENT_H
