#ifndef LAYOUT_H
#define LAYOUT_H


#include <xlib/client.h>
#include <xlib/monitor.h>


/* types */
typedef struct layout_t{
	char const *symbol;
	void (*layout_arrange)(void);
} layout_t;


/* prototypes */
client_t *nexttiled(client_t *c);
void layout_arrange(void);

void layout_tile(void);
void layout_monocle(void);


#endif // LAYOUT_H
