#ifndef BUTTONS_H
#define BUTTONS_H


#include <core/client.h>
#include <utils/utils.h>
#include <commands.h>


/* macros */
#define BUTTON_N(_id, _button, _mods, _loc, _action, ...) \
	static buttonmap_t const button_##_id \
		linker_array("buttons") = { \
			.loc = _loc, \
			.mods = _mods, \
			.button = _button, \
			.action = _action, \
			.arg = { __VA_ARGS__ }, \
		}

#define BUTTON(button, mods, loc, action, ...)	UNIQUE(BUTTON_N, __COUNTER__, button, mods, loc, action, __VA_ARGS__)


/* types */
typedef enum{
	BLOC_UNKNOWN = -1,
	BLOC_ROOT = 0,
	BLOC_CLIENT,
	BLOC_LAUNCHER,
	BLOC_TAGBAR,
	BLOC_LAYOUT,
} button_loc_t;

typedef struct{
	button_loc_t loc;

	unsigned int mods;
	unsigned int button;

	cmd_t action;
	cmd_arg_t arg;
} buttonmap_t;


/* global functions */
void buttons_register(client_t *c);
void button_handle(button_loc_t loc, unsigned int button, unsigned int mods);


/* external variables */
extern buttonmap_t __start_buttons[],
				   __stop_buttons[];


#endif // BUTTONS_H
