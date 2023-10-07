#ifndef WINDOW_H
#define WINDOW_H


#include <stdbool.h>
#include <X11/X.h>


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
} win_hints_t;

typedef struct{
	int x,
		y,
		width,
		height;

	int border_width;
} win_geom_t;


/* prototypes */
void win_show(Window win, win_geom_t *geom);
void win_hide(Window win, win_geom_t *geom);

void win_kill(Window win);
void win_release(Window win, win_geom_t *original);

void win_configure(Window win, win_geom_t *geom);
void win_resize(Window win, win_geom_t *geom, win_hints_t *hints);
void win_set_state(Window win, long state);
bool win_send_event(Window win, Atom proto);

void win_update_wmhints(Window win, win_hints_t *hints, bool isfocused);
void win_update_sizehints(Window win, win_hints_t *hints);


#endif // WINDOW_H
