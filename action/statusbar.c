#include <actions.h>
#include <core/statusbar.h>


/* global functions */
void action_statusbar_toggle(action_arg_t const *arg){
	statusbar_toggle();
}
