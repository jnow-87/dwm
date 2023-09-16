#ifndef LAYOUT_H
#define LAYOUT_H


#include <dwm.h>


/* prototypes */
client_t *nexttiled(client_t *c);
void arrange(monitor_t *m);
void arrangemon(monitor_t *m);

void tile(monitor_t *m);
void monocle(monitor_t *m);


#endif // LAYOUT_H
