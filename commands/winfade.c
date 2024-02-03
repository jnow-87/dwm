#include <config/config.h>
#include <unistd.h>
#include <core/client.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/tags.h>
#include <xlib/window.h>
#include <xlib/xlib.h>
#include <utils/list.h>
#include <commands.h>


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
static void fade(size_t n, unsigned int fades);
static void move(client_t *c, int dx, int dy);
static delta_t delta(int win_low, int win_high, int mon_low, int mon_high);


/* global functions */
void cmd_winfade_add(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0)
		return;

	c->fades ^= arg->ui;
}

void cmd_winfade_fade(cmd_arg_t const *arg){
	size_t n = 0;
	client_t *c;


	list_for_each(dwm.stack, c){
		if(c->fades & arg->ui)
			n++;
	}

	fade(n, arg->ui);
}


/* local functions */
static void fade(size_t n, unsigned int fades){
	size_t i;
	client_t *clients[n];
	win_geom_t *geom;
	delta_t dx[n],
			dy[n];
	fade_t dir;
	client_t *c;
	monitor_t *m;


	if(n == 0)
		return;

	i = 0;

	list_for_each(dwm.stack, c){
		if((c->fades & fades) == 0)
			continue;

		clients[i++] = c;
		c->geom_store = c->geom;
	}

	dir = win_visible(clients[0]->win, &clients[0]->geom) ? FADE_OUT : FADE_IN;

	/* calculate x, y movement per client */
	for(i=0; i<n; i++){
		geom = &clients[i]->geom;
		m = monitor_from_client(clients[i]);

		dx[i] = delta(geom->x, geom->x + geom->width + 2 * geom->border_width, m->x, m->x + m->width);
		dy[i] = delta(geom->y, geom->y + geom->height + 2 * geom->border_width, m->y, m->y + m->height);

		// only move to the border with the min distance
		if(dx[i].min < dy[i].min && dy[i].min > 0)		dy[i].delta = 0;
		else if(dy[i].min < dx[i].min && dx[i].min > 0)	dx[i].delta = 0;

		dx[i].delta /= (ssize_t)CONFIG_FADE_STEPS * dir;
		dy[i].delta /= (ssize_t)CONFIG_FADE_STEPS * dir;
	}

	/* update windows */
	// fade-in prologue
	for(i=0; i<n && dir==FADE_IN; i++){
		c = clients[i];

		tags_set(&c->tags, dwm.tag_mask);
		move(c, -dx[i].delta * CONFIG_FADE_STEPS, -dy[i].delta * CONFIG_FADE_STEPS);
		win_show(c->win);
		win_focus(c->win);
	}

	// fade
	for(i=1; i<=CONFIG_FADE_STEPS; i++){
		for(size_t j=0; j<n; j++)
			move(clients[j], dx[j].delta, dy[j].delta);

		xlib_sync();
		usleep(CONFIG_FADE_DELAY_MS * 1000);
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

	win_resize(c->win, geom, 0x0);
}

static delta_t delta(int win_low, int win_high, int mon_low, int mon_high){
	if(win_low - mon_low < mon_high - win_high)
		return (delta_t){ .delta = win_high - mon_low, .min = win_low - mon_low };

	return (delta_t){ .delta = win_low - mon_high, .min = mon_high - win_high };
}
