#ifndef KEYLOCK_H
#define KEYLOCK_H


#include <stdbool.h>
#include <xlib/input.h>


/* prototypes */
bool keylock_active(void);
bool keylock_key_match(keysym_t sym, unsigned int mods);


#endif // KEYLOCK_H
