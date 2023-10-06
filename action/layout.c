#include <config.h>
#include <dwm.h>
#include <action.h>
#include <client.h>
#include <statusbar.h>


/* global functions */
void action_setlayout(action_arg_t const *arg){
	if(arg == 0x0 || arg->v == 0x0){
		for(unsigned int i=0; i<nlayouts; i++){
			if(dwm.layout == layouts + i){
				i = (i + 1 < nlayouts) ? i + 1 : 0;
				dwm.layout = layouts + i;
				break;
			}
		}
	}
	else
		dwm.layout = (layout_t*)arg->v;

	if(dwm.focused)
		arrange();

	statusbar_update();
}
