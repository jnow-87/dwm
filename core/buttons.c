#include <core/dwm.h>
#include <core/buttons.h>
#include <utils/utils.h>


/* global functions */
void buttons_handle(click_t click, unsigned int button, unsigned int mods){
	button_map_t *map;


	config_for_each(buttons, map){
		if(click == map->click && map->func && map->button == button && CLEANMODS(map->mask) == CLEANMODS(mods))
			map->func(&map->arg);
	}
}
