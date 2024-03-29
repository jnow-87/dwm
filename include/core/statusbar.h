#ifndef STATUSBAR_H
#define STATUSBAR_H


#include <stdbool.h>
#include <core/buttons.h>
#include <xlib/window.h>


/* types */
typedef struct{
	bool hidden;

	window_t win;
	win_geom_t geom;

	int fd_timer;

	struct {
		size_t tags_begin,
			   tags_end,
			   layout_begin,
			   layout_end;
	} pos;
} statusbar_t;


/* prototypes */
int statusbar_init(unsigned int height);
void statusbar_destroy(void);

void statusbar_update(void);
void statusbar_raise(void);

void statusbar_toggle(void);

button_loc_t statusbar_element(int x, int y);


#endif // STATUSBAR_H
