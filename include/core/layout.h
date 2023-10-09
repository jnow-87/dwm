#ifndef LAYOUT_H
#define LAYOUT_H


#include <core/client.h>
#include <xlib/window.h>


/* types */
typedef struct layout_t{
	char const *symbol;
	void (*arrange)(void);
} layout_t;


/* prototypes */
client_t *layout_next_tiled(client_t *c);
void layout_arrange(void);

void layout_tile(void);
void layout_monocle(void);


#endif // LAYOUT_H
