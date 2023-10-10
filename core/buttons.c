#include <core/buttons.h>
#include <core/client.h>
#include <core/dwm.h>
#include <xlib/input.h>
#include <utils/utils.h>


/* global functions */
void buttons_register(client_t *c){
	buttonmap_t *button;


	input_buttons_release(c->win);

	config_for_each(buttons, button){
		if(button->loc == BLOC_CLIENT)
			input_button_register(c->win, button->button, button->mods);
	}
}

void button_handle(button_loc_t loc, unsigned int button, unsigned int mods){
	buttonmap_t *b;


	config_for_each(buttons, b){
		if(loc == b->loc && b->action && b->button == button && CLEANMODS(b->mods) == CLEANMODS(mods))
			b->action(&b->arg);
	}
}
