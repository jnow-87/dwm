#ifndef KEYLOCK_H
#define KEYLOCK_H


#include <stdbool.h>
#include <core/client.h>
#include <xlib/input.h>


/* prototypes */
void keylock_set(client_t *c);
bool keylock_active(void);
bool keylock_key_match(keysym_t sym, unsigned int mods);


#endif // KEYLOCK_H
