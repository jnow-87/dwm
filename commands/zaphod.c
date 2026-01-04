#include <core/dwm.h>
#include <core/statusbar.h>
#include <commands.h>


/* global functions */
void cmd_zaphod_toggle(cmd_arg_t const *arg){
	dwm.zaphod_en = !dwm.zaphod_en;
	statusbar_update();
}
