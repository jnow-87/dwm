#ifndef CLIENTSTACK_H
#define CLIENTSTACK_H


#include <stdbool.h>
#include <core/client.h>


/* types */
typedef enum{
	CYCLE_START = 0,
	CYCLE_CONT,
	CYCLE_END,
} cycle_state_t;


/* prototypes */
client_t *clientstack_cycle(int dir, cycle_state_t state, bool ignore_zaphod);
void clientstack_refocus(void);
void clientstack_focus(client_t *c, bool restack);


#endif // CLIENTSTACK_H
