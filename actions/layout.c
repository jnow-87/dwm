#include <core/dwm.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <actions.h>


/* global functions */
void action_layout_select(action_arg_t const *arg){
	layout_t *l;


	if(arg == 0x0 || arg->v == 0x0){
		config_for_each(layouts, l){
			if(l == dwm.layout){
				dwm.layout = (l + 1 == __stop_layouts) ? __start_layouts : l + 1;
				break;
			}
		}
	}
	else
		dwm.layout = (layout_t*)arg->v;

	layout_arrange();
	statusbar_update();
}
