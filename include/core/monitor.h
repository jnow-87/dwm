#ifndef MONITOR_H
#define MONITOR_H


#include <xlib/window.h>


/* types */
typedef struct monitor_t{
	struct monitor_t *prev,
					 *next;

	int x,
		y,
		width,
		height;
} monitor_t;


/* prototypes */
void monitor_discover(void);
void monitor_cleanup(void);

monitor_t *monitor_create(int x, int y, int width, int height);
void monitor_destroy(monitor_t *mon);

monitor_t *monitor_by_geom(win_geom_t *geom);


#endif // MONITOR_H
