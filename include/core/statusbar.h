#ifndef STATUSBAR_H
#define STATUSBAR_H


#include <stdbool.h>
#include <xlib/window.h>
#include <core/buttons.h>


/* types */
typedef struct{
	char status[256];
	bool hidden;

	window_t win;
	win_geom_t geom;
} statusbar_t;


/* prototypes */
void statusbar_init(unsigned int height);
void statusbar_destroy(void);

void statusbar_update(void);
void statusbar_raise(void);

void statusbar_toggle(void);

click_t statusbar_element(int x, int y);


#endif // STATUSBAR_H
