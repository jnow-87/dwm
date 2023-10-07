#include <config/config.h>
#include <xlib/window.h>
#include <core/scheme.h>
#include <core/dwm.h>
#include <xlib/atoms.h>
#include <config.h>


/* local/static prototypes */
static bool visible(void);


/* global functions */
void statusbar_init(unsigned int height){
	statusbar_t *bar = &dwm.statusbar;
	monitor_t *m = dwm.mons;


	bar->height = height;
	bar->y = CONFIG_STATUSBAR_TOP ? m->y : m->y + m->height - height;
	bar->status[0] = 0;
	bar->win = XCreateWindow(
		dwm.dpy,
		dwm.root,
		m->x,
		bar->y,
		m->width,
		height,
		0,
		DefaultDepth(dwm.dpy, dwm.screen),
		CopyFromParent,
		DefaultVisual(dwm.dpy, dwm.screen),
		CWOverrideRedirect | CWBackPixmap | CWEventMask,
		&(XSetWindowAttributes){
			.override_redirect = True,
			.background_pixmap = ParentRelative,
			.event_mask = ButtonPressMask | ExposureMask
		}
	);

	XDefineCursor(dwm.dpy, bar->win, dwm.cursor[CurNormal]->cursor);
	XMapRaised(dwm.dpy, bar->win);
	XSetClassHint(dwm.dpy, bar->win, &(XClassHint){"dwm", "dwm"});
}

void statusbar_destroy(void){
	statusbar_t *bar = &dwm.statusbar;


	XUnmapWindow(dwm.dpy, bar->win);
	XDestroyWindow(dwm.dpy, bar->win);
}

void statusbar_update(void){
	statusbar_t *bar = &dwm.statusbar;
	monitor_t *m = dwm.mons;
	int x,
		w,
		status_width;


	if(!visible())
		return;

	statusbar_raise();

	atoms_text_prop(dwm.root, XA_WM_NAME, dwm.statusbar.status, sizeof(dwm.statusbar.status));
	status_width = TEXTW(bar->status) - dwm.lrpad + 2; // 2px right padding

	/* draw status spacer */
	w = TEXTW(CONFIG_STATUSBAR_SPACER_RIGHT) - dwm.lrpad;
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeSpacer]);
	x = gfx_text(dwm.gfx, m->width - status_width - w, 0, w, bar->height, 0, CONFIG_STATUSBAR_SPACER_RIGHT, 0);

	/* draw status text */
	// draw status first so it can be overdrawn by tags later
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeNorm]);
	gfx_text(dwm.gfx, x, 0, status_width, bar->height, 0, bar->status, 0);
	status_width += w;

	/* draw tags */
	x = 0;

	for(unsigned int i=0; i<ntags; i++){
		w = TEXTW(tags[i]);
		gfx_setscheme(dwm.gfx, dwm.scheme[(dwm.tag_mask & (1 << i)) ? SchemeSel : SchemeNorm]);
		gfx_text(dwm.gfx, x, 0, w, bar->height, dwm.lrpad / 2, tags[i], 0);

		x += w;
	}

	/* draw layout symbol */
	w = TEXTW(dwm.layout->symbol);
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeNorm]);
	x = gfx_text(dwm.gfx, x, 0, w, bar->height, dwm.lrpad / 2, dwm.layout->symbol, 0);

	/* draw layout spacer */
	w = TEXTW(CONFIG_STATUSBAR_SPACER_LEFT) - dwm.lrpad;
	gfx_setscheme(dwm.gfx, dwm.scheme[SchemeSpacer]);
	x = gfx_text(dwm.gfx, x, 0, w, bar->height, 0, CONFIG_STATUSBAR_SPACER_LEFT, 0);

	/* draw space */
	if((w = m->width - status_width - x) > bar->height){
		gfx_setscheme(dwm.gfx, dwm.scheme[SchemeSel]);
		gfx_rect(dwm.gfx, x, 0, w, bar->height, 1, 1);
	}

	gfx_map(dwm.gfx, bar->win, 0, 0, m->width, bar->height);
}

void statusbar_raise(void){
	// ensure the statusbar is on top of other windows
	XRaiseWindow(dwm.dpy, dwm.statusbar.win);
}

void statusbar_toggle(void){
	statusbar_t *bar = &dwm.statusbar;


	if(!visible()){
		XMoveResizeWindow(dwm.dpy, bar->win, dwm.mons->x, bar->y, dwm.mons->width, bar->height);
		XMapRaised(dwm.dpy, bar->win);
	}
	else
		XUnmapWindow(dwm.dpy, bar->win);
}


/* local functions */
static bool visible(void){
	XWindowAttributes wa;


	if(!XGetWindowAttributes(dwm.dpy, dwm.statusbar.win, &wa) || wa.map_state == IsUnmapped)
		return false;

	return true;
}
