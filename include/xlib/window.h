#ifndef WINDOW_H
#define WINDOW_H


#include <stdbool.h>
#include <X11/X.h>
#include <xlib/gfx.h>


/* types */
typedef Window window_t;

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

typedef struct{
	bool override_redirect;
	int map_state;

	win_geom_t geom;
} win_attr_t;


/* prototypes */
window_t win_create(win_geom_t *geom, cursor_type_t cursor, char *class, bool override_redirect);
void win_destroy(window_t win);

void win_init(window_t win, win_geom_t *geom, win_hints_t *hints);
void win_kill(window_t win);
void win_release(window_t win, win_geom_t *original);

void win_configure(window_t win, win_geom_t *geom);
void win_resize(window_t win, win_geom_t *geom, win_hints_t *hints);
void win_raise(window_t win);
void win_set_state(window_t win, long state);
bool win_send_event(window_t win, Atom proto);

void win_show(window_t win, win_geom_t *geom);
void win_hide(window_t win, win_geom_t *geom);
void win_focus(window_t win);
void win_unfocus(window_t win);

int win_get_attr(window_t win, win_attr_t *attr);
long win_get_state(window_t w);
window_t win_get_transient(window_t win);
void win_update_wmhints(window_t win, win_hints_t *hints, bool isfocused);
void win_update_sizehints(window_t win, win_hints_t *hints);


#endif // WINDOW_H
