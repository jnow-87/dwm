#ifndef KEYS_H
#define KEYS_H


#include <stdbool.h>
#include <xlib/input.h>


/* types */
typedef void (*cycle_callback_t)(void);


/* prototypes */
int keys_init(void);
void keys_cleanup(void);

void keys_handle(keysym_t sym, unsigned int mods);

void keys_cycle_start(cycle_callback_t complete);
void keys_cycle_complete(void);
bool keys_cycle_active(void);


#endif // KEYS_H
