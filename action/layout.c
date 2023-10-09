#include <core/dwm.h>
#include <core/statusbar.h>
#include <actions.h>
#include <config.h>


/* global functions */
void action_layout_select(action_arg_t const *arg){
	if(arg == 0x0 || arg->v == 0x0){
		for(size_t i=0; i<nlayouts; i++){
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
		layout_arrange();

	statusbar_update();
}
