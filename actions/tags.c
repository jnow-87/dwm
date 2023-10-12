#include <core/dwm.h>
#include <core/tags.h>
#include <actions.h>


/* global functions */
void action_tags_view(action_arg_t const *arg){
	tags_set(&dwm.tag_mask, arg->ui);
}

void action_tags_toggle(action_arg_t const *arg){
	tags_toggle(&dwm.tag_mask, arg->ui);
}

void action_tags_client_set(action_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	tags_set(&c->tags, arg->ui);
}

void action_tags_client_toggle(action_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	tags_toggle(&c->tags, arg->ui);
}
