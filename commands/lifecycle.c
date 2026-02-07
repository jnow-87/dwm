#include <core/dwm.h>
#include <utils/menu.h>
#include <commands.h>


/* global functions */
void cmd_lifecycle(cmd_arg_t const *arg){
	switch(menu((char const *[]){"restart", "quit"}, 2)){
	case 0:	dwm.state = DWM_RESTART; break;
	case 1:	dwm.state = DWM_SHUTDOWN; break;
	}
}
