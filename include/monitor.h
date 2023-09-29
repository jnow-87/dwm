#ifndef MONITOR_H
#define MONITOR_H


#include <X11/X.h>


/* incomplete types */
struct client_t;


/* types */
typedef struct monitor_t{
	int x,
		y,
		width,
		height;

	struct monitor_t *next;
} monitor_t;


/* prototypes */
monitor_t *createmon(void);
monitor_t *client_to_monitor(struct client_t *c);
void cleanupmon(monitor_t *mon);
void restack(void);


#endif // MONITOR_H
