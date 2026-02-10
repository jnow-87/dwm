#ifndef KEYS_H
#define KEYS_H


#include <stdbool.h>
#include <xlib/input.h>
#include <commands.h>


/* macros */
#define KEY_INITIALISER() (keymap_t){ \
	.keysym = NoSymbol, \
	.mods = 0, \
	.action = 0x0, \
	.arg = { 0 }, \
}


/* types */
typedef void (*cycle_callback_t)(void);

typedef struct{
	keysym_t keysym;
	unsigned int mods;

	cmd_action_t action;
	cmd_arg_t arg;
} keymap_t;


/* prototypes */
int keys_init(void);
void keys_cleanup(void);

int keys_register(void);
void keys_handle(keysym_t sym, unsigned int mods);
bool keys_registered(keymap_t *key, keysym_t sym, unsigned int mods);

void keys_cycle_start(cycle_callback_t complete);
void keys_cycle_complete(void);
bool keys_cycle_active(void);

int key_verify(keymap_t *key);


#endif // KEYS_H
