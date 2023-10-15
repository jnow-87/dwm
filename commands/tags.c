#include <core/dwm.h>
#include <core/tags.h>
#include <commands.h>


/* global functions */
void cmd_tags_view(cmd_arg_t *arg){
	tags_set(&dwm.tag_mask, arg->ui);
}

void cmd_tags_toggle(cmd_arg_t *arg){
	tags_toggle(&dwm.tag_mask, arg->ui);
}

void cmd_tags_client_set(cmd_arg_t *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	tags_set(&c->tags, arg->ui);
}

void cmd_tags_client_toggle(cmd_arg_t *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	tags_toggle(&c->tags, arg->ui);
}
