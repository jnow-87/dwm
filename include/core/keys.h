#ifndef KEYS_H
#define KEYS_H


#include <stdbool.h>
#include <xlib/input.h>
#include <utils/utils.h>
#include <commands.h>


/* macros */
#define KEY_N(_id, _keysym, _mods, _action, ...) \
	static keymap_t const key_##_id \
		linker_array("keys") = { \
			.keysym = _keysym, \
			.mods = _mods, \
			.action = _action, \
			.arg = { __VA_ARGS__ }, \
		}

#define KEY(keysym, mods, action, ...)	UNIQUE(KEY_N, __COUNTER__, keysym, mods, action, __VA_ARGS__)


/* types */
typedef void (*cycle_callback_t)(void);

typedef struct{
	keysym_t keysym;
	unsigned int mods;

	cmd_t action;
	cmd_arg_t arg;
} keymap_t;


/* prototypes */
int keys_init(void);
void keys_cleanup(void);

int keys_register(void);
void keys_handle(keysym_t sym, unsigned int mods);

void keys_cycle_start(cycle_callback_t complete);
void keys_cycle_complete(void);
bool keys_cycle_active(void);


/* external variables */
extern keymap_t __start_keys[],
				__stop_keys[];


#endif // KEYS_H
