#include <config/config.h>
#include <core/buttons.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/tags.h>
#include <xlib/atoms.h>
#include <xlib/gfx.h>
#include <xlib/window.h>


/* macros */
#define PADDING		CONFIG_STATUSBAR_PADDING
#define TEXTW(s)	(gfx_text_width(dwm.gfx, s) + PADDING)


/* global functions */
void statusbar_init(unsigned int height){
	statusbar_t *bar = &dwm.statusbar;
	monitor_t *m = dwm.mons;


	bar->hidden = false;
	bar->status[0] = 0;
	bar->geom.x = m->x;
	bar->geom.y = CONFIG_STATUSBAR_TOP ? m->y : m->y + m->height - height;
	bar->geom.width = m->width;
	bar->geom.height = height;
	bar->geom.border_width = 0;

	bar->win = win_create(&bar->geom, CUR_NORM, "dwm");
	win_raise(bar->win);

	statusbar_update();
}

void statusbar_destroy(void){
	win_destroy(dwm.statusbar.win);
}

void statusbar_update(void){
	statusbar_t *bar = &dwm.statusbar;
	monitor_t *m = dwm.mons;
	int bar_height = bar->geom.height;
	char **tag;
	size_t i;
	int x,
		w,
		status_width;


	if(bar->hidden)
		return;

	statusbar_raise();

	atoms_text_prop(dwm.root, XA_WM_NAME, bar->status, sizeof(bar->status));
	status_width = TEXTW(bar->status) - PADDING + 2;

	/* draw status spacer */
	w = TEXTW(CONFIG_STATUSBAR_SPACER_RIGHT) - PADDING;
	x = gfx_text(dwm.gfx, m->width - status_width - w, 0, w, bar_height, SCM_SPACER, 0, CONFIG_STATUSBAR_SPACER_RIGHT, 0);

	/* draw status text */
	// draw status first so it can be overdrawn by tags later
	gfx_text(dwm.gfx, x, 0, status_width, bar_height, SCM_NORM, 0, bar->status, 0);
	status_width += w;

	/* draw tags */
	i = 0;
	x = 0;

	config_for_each(tags, tag){
		w = TEXTW(*tag);
		gfx_text(dwm.gfx, x, 0, w, bar_height, (dwm.tag_mask & (1 << i)) ? SCM_FOCUS : SCM_NORM, PADDING / 2, *tag, 0);

		i++;
		x += w;
	}

	/* draw layout symbol */
	w = TEXTW(dwm.layout->symbol);
	x = gfx_text(dwm.gfx, x, 0, w, bar_height, SCM_NORM, PADDING / 2, dwm.layout->symbol, 0);

	/* draw layout spacer */
	w = TEXTW(CONFIG_STATUSBAR_SPACER_LEFT) - PADDING;
	x = gfx_text(dwm.gfx, x, 0, w, bar_height, SCM_SPACER, 0, CONFIG_STATUSBAR_SPACER_LEFT, 0);

	/* draw space */
	if((w = m->width - status_width - x) > bar_height){
		gfx_rect(dwm.gfx, x, 0, w, bar_height, SCM_FOCUS, 1, 1);
	}

	gfx_map(dwm.gfx, bar->win, 0, 0, m->width, bar_height);
}

void statusbar_raise(void){
	// ensure the statusbar is on top of other windows
	win_raise(dwm.statusbar.win);
}

void statusbar_toggle(void){
	statusbar_t *bar = &dwm.statusbar;


	if(bar->hidden)	win_show(bar->win, &bar->geom);
	else			win_hide(bar->win, &bar->geom);

	bar->hidden = !bar->hidden;
}

click_t statusbar_element(int x, int y){
	unsigned int pos = 0;
	statusbar_t *bar = &dwm.statusbar;
	char **tag;


	if(y < bar->geom.y || y >= bar->geom.y + bar->geom.height)
		return CLK_UNKNOWN;

	config_for_each(tags, tag){
		pos += TEXTW(*tag);

		if(pos > x)
			return CLK_TAGBAR;
	}

	if(x < pos + TEXTW(dwm.layout->symbol))
		return CLK_LAYOUT;

	if(x > dwm.mons->width - (int)TEXTW(bar->status))
		return CLK_STATUS;

	return CLK_UNKNOWN;
}
