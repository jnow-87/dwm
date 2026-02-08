#ifndef BUTTONS_H
#define BUTTONS_H


#include <core/client.h>
#include <utils/utils.h>
#include <commands.h>


/* macros */
#define BUTTON_INITIALISER() (buttonmap_t){ \
	.loc = BLOC_UNKNOWN, \
	.mods = 0, \
	.button = 0, \
	.action = 0x0, \
	.arg = { 0 }, \
}


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

	cmd_action_t action;
	cmd_arg_t arg;
} buttonmap_t;


/* global functions */
void buttons_register(client_t *c);
void button_handle(button_loc_t loc, unsigned int button, unsigned int mods);
int button_verify(buttonmap_t *btn);


#endif // BUTTONS_H
