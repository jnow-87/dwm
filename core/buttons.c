#include <config.h>
#include <core/dwm.h>
#include <core/buttons.h>
#include <xlib/input.h>


/* global functions */
void buttons_handle(click_t click, unsigned int button, unsigned int mods){
	for(size_t i=0; i<nbuttons; i++){
		if(click == buttons[i].click && buttons[i].func && buttons[i].button == button && CLEANMODS(buttons[i].mask) == CLEANMODS(mods))
			buttons[i].func(&buttons[i].arg);
	}
}
