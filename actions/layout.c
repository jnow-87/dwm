#include <core/dwm.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <utils/menu.h>
#include <actions.h>


/* global functions */
void action_layout_select(action_arg_t const *arg){
	int n = 0;
	char const *names[__stop_layouts - __start_layouts];
	layout_t *l;


	if(arg == 0x0 || arg->v == 0x0){
		config_for_each(layouts, l)
			names[n++] = l->name;

		n = menu(names, n);

		if(n == -1)
			return;

		dwm.layout = __start_layouts + n;
	}
	else
		dwm.layout = (layout_t*)arg->v;

	layout_arrange();
	statusbar_update();
}
