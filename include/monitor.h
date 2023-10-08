#ifndef MONITOR_H
#define MONITOR_H


#include <client.h>


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
monitor_t *monitor_create(int x, int y, int width, int height);
void monitor_destroy(monitor_t *mon);

monitor_t *monitor_from_client(client_t *c);

void monitor_restack(void);


#endif // MONITOR_H
