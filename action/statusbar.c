#include <core/actions.h>
#include <core/statusbar.h>


/* global functions */
void action_togglebar(action_arg_t const *arg){
	statusbar_toggle();
}
