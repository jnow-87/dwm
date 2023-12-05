#include <config/config.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <core/buttons.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/tags.h>
#include <utils/timer.h>
#include <xlib/atoms.h>
#include <xlib/gfx.h>
#include <xlib/window.h>


/* macros */
#define PADDING		CONFIG_STATUSBAR_PADDING


/* local/static prototypes */
static void draw_left(char const *s, scheme_id_t scheme, int padding, int *x);
static void draw_right(char const *s, scheme_id_t scheme, int padding, int *x);
static void datetime(char *s, size_t n);
static void statd_refresh(bool indicate_error);

static int timer_hdlr(void);


/* global functions */
int statusbar_init(unsigned int height){
	statusbar_t *bar = &dwm.statusbar;
	monitor_t *m = dwm.mons;


	bar->hidden = false;
	bar->geom.x = m->x;
	bar->geom.y = CONFIG_STATUSBAR_TOP ? m->y : m->y + m->height - height;
	bar->geom.width = m->width;
	bar->geom.height = height;
	bar->geom.border_width = 0;

	bar->win = win_create(&bar->geom, CUR_NORM, "dwm", true);
	bar->fd_timer = timer_init();

	if(bar->fd_timer == -1)
		return -1;

	if(dwm_hdlr_add(bar->fd_timer, timer_hdlr) != 0)
		return -1;

	timer_set(bar->fd_timer, 60000);
	win_raise(bar->win);

	statusbar_update();

	return 0;
}

void statusbar_destroy(void){
	timer_destroy(dwm.statusbar.fd_timer);
	win_destroy(dwm.statusbar.win);
}

void statusbar_update(void){
	statusbar_t *bar = &dwm.statusbar;
	char **tag;
	size_t i;
	int x;
	char s[256];


	if(bar->hidden)
		return;

	statusbar_raise();

	/* background */
	x = dwm.mons->width;
	gfx_rect(dwm.gfx, 0, 0, x, bar->geom.height, SCM_FOCUS, 1, 1);

	/* right side */
	// date and time
	datetime(s, sizeof(s));
	draw_right(s, SCM_NORM, PADDING, &x);
	draw_right(CONFIG_STATUSBAR_SPACER_RIGHT, SCM_SPACER_NORM, 0, &x);

	// status
	if(win_get_name(dwm.root, s, sizeof(s)) != 0 || *s == 0)
		strcpy(s, "no status info");

	draw_right(s, SCM_STATUS, PADDING, &x);
	draw_right(CONFIG_STATUSBAR_SPACER_RIGHT, SCM_SPACER_STATUS, 0, &x);

	/* left side */
	x = 0;

	// tags
	i = 0;

	config_for_each(tags, tag){
		if(dwm.tag_mask & (1 << i))
			break;

		i++;
	}

	i = tags_selected(dwm.tag_mask);

	if(i > 1)	snprintf(s, sizeof(s), "%s [%zu]", CONFIG_STATUSBAR_TAGS_MULTI, i);
	else		snprintf(s, sizeof(s), "%s", *tag);
	
	draw_left(s, SCM_NORM, PADDING, &x);
	draw_left(CONFIG_STATUSBAR_SPACER_LEFT, SCM_SPACER_NORM, 0, &x);

	// layout symbol
	draw_left(dwm.layout->symbol, SCM_STATUS, PADDING, &x);
	draw_left(CONFIG_STATUSBAR_SPACER_LEFT, SCM_SPACER_STATUS, 0, &x);

	/* sync */
	gfx_map(dwm.gfx, bar->win, 0, 0, dwm.mons->width, bar->geom.height);
}

void statusbar_raise(void){
	// ensure the statusbar is on top of other windows
	win_raise(dwm.statusbar.win);
}

void statusbar_toggle(void){
	statusbar_t *bar = &dwm.statusbar;


	if(bar->hidden){
		statd_refresh(false);
		win_show(bar->win);
	}
	else
		win_hide(bar->win);

	bar->hidden = !bar->hidden;
}

button_loc_t statusbar_element(int x, int y){
	unsigned int pos = 0;
	statusbar_t *bar = &dwm.statusbar;
	char **tag;


	if(y < bar->geom.y || y >= bar->geom.y + bar->geom.height)
		return BLOC_UNKNOWN;

	config_for_each(tags, tag){
		pos += gfx_text_width(dwm.gfx, *tag) + PADDING;

		if(pos > x)
			return BLOC_TAGBAR;
	}

	if(x < pos + gfx_text_width(dwm.gfx, dwm.layout->symbol) + PADDING)
		return BLOC_LAYOUT;

	return BLOC_UNKNOWN;
}


/* local functions */
static void draw_left(char const *s, scheme_id_t scheme, int padding, int *x){
	int w;


	w = gfx_text_width(dwm.gfx, s) + padding;
	gfx_text(dwm.gfx, *x, 0, w, dwm.statusbar.geom.height, scheme, padding / 2, s, 0);
	*x += w;
}

static void draw_right(char const *s, scheme_id_t scheme, int padding, int *x){
	int w;


	w = gfx_text_width(dwm.gfx, s) + padding;
	*x -= w;
	gfx_text(dwm.gfx, *x, 0, w, dwm.statusbar.geom.height, scheme, padding / 2, s, 0);
}

static void datetime(char *s, size_t n){
	time_t t;


	time(&t);
	strftime(s, n, "%d %b, %H:%M", localtime(&t));
	s[n - 1] = 0;
}

static void statd_refresh(bool indicate_error){
	statusbar_t *bar = &dwm.statusbar;


	if(system("statdctrl refresh") == 0)
		return;

	win_set_name(dwm.root, "");

	// show statusbar to indicate dead statd
	if(indicate_error && bar->hidden){
		win_show(bar->win);
		bar->hidden = false;
	}
}

static int timer_hdlr(void){
	timer_ack(dwm.statusbar.fd_timer);

	statd_refresh(true);
	statusbar_update();

	return 0;
}
