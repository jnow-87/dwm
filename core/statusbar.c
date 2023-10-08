#include <config/config.h>
#include <config.h>
#include <core/buttons.h>
#include <core/scheme.h>
#include <core/dwm.h>
#include <core/statusbar.h>
#include <xlib/atoms.h>
#include <xlib/window.h>


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

	bar->win = win_create(&bar->geom, CurNormal, "dwm");
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
	int x,
		w,
		status_width;


	if(bar->hidden)
		return;

	statusbar_raise();

	atoms_text_prop(dwm.root, XA_WM_NAME, bar->status, sizeof(bar->status));
	status_width = TEXTW(bar->status) - dwm.lrpad + 2; // 2px right padding

	/* draw status spacer */
	w = TEXTW(CONFIG_STATUSBAR_SPACER_RIGHT) - dwm.lrpad;
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeSpacer]);
	x = gfx_text(dwm.gfx, m->width - status_width - w, 0, w, bar_height, 0, CONFIG_STATUSBAR_SPACER_RIGHT, 0);

	/* draw status text */
	// draw status first so it can be overdrawn by tags later
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeNorm]);
	gfx_text(dwm.gfx, x, 0, status_width, bar_height, 0, bar->status, 0);
	status_width += w;

	/* draw tags */
	x = 0;

	for(unsigned int i=0; i<ntags; i++){
		w = TEXTW(tags[i]);
		gfx_setscheme(dwm.gfx, dwm.scheme[(dwm.tag_mask & (1 << i)) ? SchemeSel : SchemeNorm]);
		gfx_text(dwm.gfx, x, 0, w, bar_height, dwm.lrpad / 2, tags[i], 0);

		x += w;
	}

	/* draw layout symbol */
	w = TEXTW(dwm.layout->symbol);
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeNorm]);
	x = gfx_text(dwm.gfx, x, 0, w, bar_height, dwm.lrpad / 2, dwm.layout->symbol, 0);

	/* draw layout spacer */
	w = TEXTW(CONFIG_STATUSBAR_SPACER_LEFT) - dwm.lrpad;
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeSpacer]);
	x = gfx_text(dwm.gfx, x, 0, w, bar_height, 0, CONFIG_STATUSBAR_SPACER_LEFT, 0);

	/* draw space */
	if((w = m->width - status_width - x) > bar_height){
		gfx_setscheme(dwm.gfx, dwm.scheme[SchemeSel]);
		gfx_rect(dwm.gfx, x, 0, w, bar_height, 1, 1);
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


	if(y < bar->geom.y || y >= bar->geom.y + bar->geom.height)
		return CLK_UNKNOWN;

	for(size_t i=0; i<ntags; i++){
		pos += TEXTW(tags[i]);

		if(pos > x)
			return CLK_TAGBAR;
	}

	if(x < pos + TEXTW(dwm.layout->symbol))
		return CLK_LAYOUT;

	if(x > dwm.mons->width - (int)TEXTW(bar->status))
		return CLK_STATUS;

	return CLK_UNKNOWN;
}
