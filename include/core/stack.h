#ifndef CLIENT_STACK_H
#define CLIENT_STACK_H


#include <stdbool.h>
#include <core/client.h>


/* types */
typedef enum{
	CYCLE_START = 0,
	CYCLE_CONT,
	CYCLE_END,
} cycle_state_t;


/* prototypes */
client_t *client_cycle(int dir, cycle_state_t state);
void client_refocus(void);
void client_focus(client_t *c, bool restack);


#endif // CLIENT_STACK_H
