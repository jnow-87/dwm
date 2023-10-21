#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <core/tags.h>
#include <commands.h>


/* local/static prototypes */
static void set(unsigned int *tags, unsigned int v);
static void toggle(unsigned int *tags, unsigned int v);


/* global functions */
void cmd_tags_view(cmd_arg_t *arg){
	set(&dwm.tag_mask, arg->ui);
}

void cmd_tags_toggle(cmd_arg_t *arg){
	toggle(&dwm.tag_mask, arg->ui);
}

void cmd_tags_client_set(cmd_arg_t *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	set(&c->tags, arg->ui);
}

void cmd_tags_client_toggle(cmd_arg_t *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	toggle(&c->tags, arg->ui);
}


/* local functions */
static void set(unsigned int *tags, unsigned int v){
	tags_set(tags, v);

	layout_arrange();
	clientstack_refocus();
	statusbar_update();
}

static void toggle(unsigned int *tags, unsigned int v){
	set(tags, v ^ *tags);
}
