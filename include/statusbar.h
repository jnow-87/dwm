#ifndef STATUSBAR_H
#define STATUSBAR_H


#include <stdbool.h>
#include <X11/X.h>


/* types */
typedef struct{
	char status[256];

	Window win;
	unsigned int height;
	int y;
} statusbar_t;


/* prototypes */
void statusbar_init(unsigned int height);
void statusbar_destroy(void);

void statusbar_update(void);

void statusbar_draw(void);
void statusbar_toggle(void);


#endif // STATUSBAR_H
