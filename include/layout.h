#ifndef LAYOUT_H
#define LAYOUT_H


#include <client.h>
#include <monitor.h>


/* types */
typedef struct layout_t{
	const char *symbol;
	void (*arrange)(monitor_t *);
} layout_t;


/* prototypes */
client_t *nexttiled(client_t *c);
void arrange(monitor_t *m);
void arrangemon(monitor_t *m);

void tile(monitor_t *m);
void monocle(monitor_t *m);


#endif // LAYOUT_H
