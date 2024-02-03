#include <core/statusbar.h>
#include <commands.h>


/* global functions */
void cmd_statusbar_toggle(cmd_arg_t const *arg){
	statusbar_toggle();
}
