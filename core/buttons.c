#include <core/buttons.h>
#include <core/client.h>
#include <core/dwm.h>
#include <utils/log.h>
#include <utils/vector.h>
#include <xlib/input.h>
#include <rc.h>


/* global functions */
void buttons_register(client_t *c){
	buttonmap_t *button;


	input_buttons_release(c->win);

	vector_for_each(&rc.buttons, button){
		if(button->loc == BLOC_CLIENT)
			input_button_register(c->win, button->button, button->mods);
	}
}

void button_handle(button_loc_t loc, unsigned int button, unsigned int mods){
	buttonmap_t *b;


	vector_for_each(&rc.buttons, b){
		if(loc == b->loc && b->action && b->button == button && CLEANMODS(b->mods) == CLEANMODS(mods))
			b->action(&b->arg);
	}
}

int button_verify(buttonmap_t *btn){
	if(btn->action == 0x0)
		return ERROR("missing action\n");

	return 0;
}
