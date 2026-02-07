#include <stdlib.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <core/tags.h>
#include <xlib/atoms.h>
#include <utils/list.h>
#include <utils/menu.h>
#include <utils/vector.h>
#include <commands.h>
#include <rc.h>


/* local/static prototypes */
static void set(unsigned int *tags, unsigned int v);


/* global functions */
int cmd_tag_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg){
	arg->i = (1 << atoi(tk));

	return 0;
}

void cmd_tags_view(cmd_arg_t const *arg){
	client_t *c;


	set(&dwm.tag_mask, arg->ui);

	netatom_set(NET_CURRENT_DESKTOP, dwm.root, (unsigned char*)&dwm.tag_mask, 1);

	list_for_each(dwm.stack, c){
		client_update_desktop(c);
	}
}

void cmd_tags_toggle(cmd_arg_t const *arg){
	cmd_tags_view(&(cmd_arg_t const){ .ui = (arg->ui ^ dwm.tag_mask) });
}

void cmd_tags_client_set(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	set(&c->tags, arg->ui);
	client_update_desktop(c);
}

void cmd_tags_client_toggle(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	cmd_tags_client_set(&(cmd_arg_t const){ .ui = (arg->ui ^ c->tags) });
}

void cmd_tags_menu(cmd_arg_t const *arg){
	int n = 0;
	char const *names[rc.tags.size];
	char **tag;


	vector_for_each(&rc.tags, tag)
		names[n++] = *tag;

	n = menu(names, n);

	if(n == -1)
		return;

	cmd_tags_view(&(cmd_arg_t const){ .ui = (1 << n) });
}


/* local functions */
static void set(unsigned int *tags, unsigned int v){
	tags_set(tags, v);

	layout_arrange();
	clientstack_refocus();
	statusbar_update();
}
