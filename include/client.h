#ifndef CLIENT_H
#define CLIENT_H


#include <X11/Xlib.h>


/* macros */
#define ISVISIBLE(c)	((c->tags & dwm.tag_mask))
#define WIDTH(c)		((c)->geom.width + 2 * (c)->geom.border_width)
#define HEIGHT(c)		((c)->geom.height + 2 * (c)->geom.border_width)


/* types */
typedef struct{
	int x,
		y,
		width,
		height;

	int border_width;
} client_geom_t;

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
	struct client_t *stack_next;

	unsigned int tags;

	Window win;
	client_geom_t geom,
				  geom_store;
	client_hints_t hints;
} client_t;


/* prototypes */
client_t *wintoclient(Window w);

void manage(Window w, XWindowAttributes *wa);
void unmanage(client_t *c, int destroyed);
void killclient(Window win);

void configure(client_t *c);

void attachstack(client_t *c);
void detachstack(client_t *c);

void focus(client_t *c);
void unfocus(client_t *c, int setfocus);
void showhide(client_t *c);

void resize(client_t *c, int x, int y, int w, int h, int interact);
void resizeclient(client_t *c, int x, int y, int w, int h);
void setclientstate(client_t *c, long state);
void setfocus(client_t *c);

int sendevent(client_t *c, Atom proto);

void updatewmhints(client_t *c);
void updatesizehints(client_t *c);


#endif // CLIENT_H
