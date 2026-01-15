#include <stdbool.h>
#include <core/client.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/keylock.h>
#include <core/statusbar.h>
#include <core/tags.h>
#include <xlib/window.h>
#include <commands.h>


/* local/static prototypes */
static void show(client_t *c, bool focus);
static void hide(client_t *c);


/* global functions */
void cmd_keylock_set(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	if(dwm.keylock != 0x0 && dwm.keylock != c)
		show(dwm.keylock, false);

	keylock_set((dwm.keylock == c) ? 0x0 : c);
}

void cmd_keylock_toggle(cmd_arg_t const *arg){
	client_t *c = dwm.keylock;


	if(c == 0x0)
		return;

	if(win_visible(c->win)) hide(c);
	else					show(c, true);
}


/* local functions */
static void show(client_t *c, bool focus){
	tags_set(&c->tags, dwm.tag_mask);
	win_show(c->win);

	if(focus)
		clientstack_focus(c, true);
}

static void hide(client_t *c){
	win_hide(c->win);
	c->tags = 0;
	clientstack_refocus();
}
