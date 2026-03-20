#include <config/config.h>
#include <stdbool.h>
#include <unistd.h>
#include <core/client.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/tags.h>
#include <utils/list.h>
#include <xlib/window.h>
#include <xlib/xlib.h>
#include <commands.h>
#include <rc.h>


/* types */
typedef enum{
	FADE_OUT = -1,
	FADE_IN = 1,
} fade_t;

typedef struct{
	int delta,
		min;
} delta_t;


/* local/static prototypes */
static void focus(client_t **clients, size_t n);
static void fade(client_t **clients, size_t n, fade_t dir);

static void move(client_t *c, int dx, int dy);
static delta_t delta(int win_low, int win_high, int mon_low, int mon_high);


/* global functions */
int cmd_winfade_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg){
	arg->i = (1 << atoi(tk));

	return 0;
}

void cmd_winfade_add(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	c->fades ^= arg->ui;
}

void cmd_winfade_fade(cmd_arg_t const *arg){
	bool visible = false,
		 focused = false;
	unsigned int fades = arg->ui;
	size_t n = 0;
	client_t *clients[dwm.nclients];
	client_t *c;


	list_for_each(dwm.stack, c){
		if((c->fades & fades) == 0)
			continue;

		clients[n++] = c;
		c->geom_store = c->geom;

		visible = visible || win_visible(c->win);
		focused = focused || ((c == dwm.stack) && visible);
	}

	if(n == 0)
		return;

	if(!focused && visible)	focus(clients, n);
	else					fade(clients, n, visible ? FADE_OUT : FADE_IN);
}


/* local functions */
static void focus(client_t **clients, size_t n){
	for(size_t i=0; i<n; i++)
		win_focus(clients[n - i - 1]->win);

	clientstack_focus(clients[0], true);
}

static void fade(client_t **clients, size_t n, fade_t dir){
	size_t i;
	win_geom_t *geom;
	delta_t dx[n],
			dy[n];
	client_t *c;
	monitor_t *m;


	/* calculate x, y movement per client */
	for(i=0; i<n; i++){
		geom = &clients[i]->geom;
		m = clients[i]->mon;

		dx[i] = delta(geom->x, geom->x + geom->width + 2 * geom->border_width, m->x, m->x + m->width);
		dy[i] = delta(geom->y, geom->y + geom->height + 2 * geom->border_width, m->y, m->y + m->height);

		// only move to the border with the min distance
		if(dx[i].min < dy[i].min && dy[i].min > 0)		dy[i].delta = 0;
		else if(dy[i].min < dx[i].min && dx[i].min > 0)	dx[i].delta = 0;

		dx[i].delta /= (ssize_t)rc.fade_steps * dir;
		dy[i].delta /= (ssize_t)rc.fade_steps * dir;
	}

	/* update windows */
	// fade-in prologue
	for(i=0; i<n && dir==FADE_IN; i++){
		c = clients[i];

		tags_set(&c->tags, dwm.tag_mask);
		move(c, -dx[i].delta * rc.fade_steps, -dy[i].delta * rc.fade_steps);
		win_show(c->win);
		win_focus(c->win);
	}

	// fade
	for(i=1; i<=rc.fade_steps; i++){
		for(size_t j=0; j<n; j++)
			move(clients[j], dx[j].delta, dy[j].delta);

		xlib_sync();
		usleep(rc.fade_delay_ms * 1000);
	}

	// fade-out epilogue
	for(i=0; i<n && dir==FADE_OUT; i++){
		c = clients[i];

		win_hide(c->win);
		c->tags = 0;
	}

	if(dir == FADE_IN)	clientstack_focus(clients[0], true);
	else				clientstack_refocus();
}

static void move(client_t *c, int dx, int dy){
	win_geom_t *geom = &c->geom_store;


	geom->x += dx;
	geom->y += dy;

	win_resize(c->win, geom);
}

static delta_t delta(int win_low, int win_high, int mon_low, int mon_high){
	if(win_low - mon_low < mon_high - win_high)
		return (delta_t){ .delta = win_high - mon_low, .min = win_low - mon_low };

	return (delta_t){ .delta = win_low - mon_high, .min = mon_high - win_high };
}
