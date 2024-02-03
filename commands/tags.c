#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <core/tags.h>
#include <xlib/atoms.h>
#include <utils/menu.h>
#include <utils/list.h>
#include <commands.h>


/* local/static prototypes */
static void set(unsigned int *tags, unsigned int v);


/* global functions */
void cmd_tags_view(cmd_arg_t *arg){
	client_t *c;


	set(&dwm.tag_mask, arg->ui);

	netatom_set(NET_CURRENT_DESKTOP, dwm.root, (unsigned char*)&dwm.tag_mask, 1);

	list_for_each(dwm.stack, c){
		client_update_desktop(c);
	}
}

void cmd_tags_toggle(cmd_arg_t *arg){
	cmd_tags_view(&(cmd_arg_t){ .ui = (arg->ui ^ dwm.tag_mask) });
}

void cmd_tags_client_set(cmd_arg_t *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	set(&c->tags, arg->ui);
	client_update_desktop(c);
}

void cmd_tags_client_toggle(cmd_arg_t *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	cmd_tags_client_set(&(cmd_arg_t const){ .ui = (arg->ui ^ c->tags) });
}

void cmd_tags_menu(cmd_arg_t *arg){
	int n = 0;
	char const *names[__stop_tags - __start_tags];
	char **tag;


	config_for_each(tags, tag){
		names[n++] = *tag;
	}

	n = menu(names, n);

	if(n == -1)
		return;

	arg->ui = 1 << n;
	cmd_tags_view(arg);
}


/* local functions */
static void set(unsigned int *tags, unsigned int v){
	tags_set(tags, v);

	layout_arrange();
	clientstack_refocus();
	statusbar_update();
}
