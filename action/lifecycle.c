#include <core/dwm.h>
#include <actions.h>


/* global functions */
void action_quit(action_arg_t const *arg){
	dwm.running = 0;
}

void action_restart(action_arg_t const *arg){
	dwm.running = -1;
}
