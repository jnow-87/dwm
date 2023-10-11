#include <config/config.h>
#include <stdint.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/xevents.h>
#include <xlib/input.h>
#include <xlib/window.h>
#include <utils/utils.h>
#include <actions.h>


/* macros */
#define SIZE_MIN	32

#define PREP_RESIZE(origin, dim, new, geom, store, mon){ \
	if((new)->dim == INT_MAX){ \
		(new)->origin = (mon)->origin; \
		(new)->dim = (mon)->dim- (geom)->border_width * 2; \
		\
		if((geom)->origin == (new)->origin && (geom)->dim == (new)->dim){ \
			(new)->origin = (store)->origin; \
			(new)->dim = (store)->dim; \
		} \
	} \
	else{ \
		(new)->origin = (geom)->origin - ((new)->dim) / 2; \
		(new)->dim += (geom)->dim; \
	} \
	\
	(new)->dim = MAX((new)->dim, 32); /* prevent too small windows */ \
	\
	/* prevent windows from moving if they would be smaller than SIZE_MIN */ \
	if((new)->dim == (geom)->dim) \
		(new)->origin = (geom)->origin; \
}


/* local/static prototypes */
static void client_cycle_complete(void);


/* global functions */
void action_client_cycle(action_arg_t const *arg){
	client_t *c;


	c = clientstack_cycle(arg->i, keys_cycle_active() ? CYCLE_CONT : CYCLE_START);

	if(c == 0x0)
		return;

	if(!keys_cycle_active())
		keys_cycle_start(client_cycle_complete);

	clientstack_focus(c, false);
	statusbar_raise();
}

void action_client_kill(action_arg_t const *arg){
	if(!dwm.focused)
		return;

	if(!win_send_event(dwm.focused->win, dwm.wmatom[WM_DELETE]))
		win_kill(dwm.focused->win);
}

void action_client_move(action_arg_t const *arg){
	int nx, ny;
	client_t *c;
	win_geom_t *geom;


	if(!(c = dwm.focused))
		return;

	geom = &c->geom;

	nx = ((int*)(arg->v))[0];
	ny = ((int*)(arg->v))[1];

	switch(nx){
	case -INT_MAX:	nx = dwm.mons->x; break;
	case INT_MAX:	nx = dwm.mons->x + dwm.mons->width - geom->width - geom->border_width*2; break;
	default:		nx += geom->x; break;
	}

	switch(ny){
	case -INT_MAX:	ny = dwm.mons->y; break;
	case INT_MAX:	ny = dwm.mons->y + dwm.mons->height - geom->height - geom->border_width*2; break;
	default:		ny += geom->y;
	}

	// TODO why do the following two lines not lead to the following glitch
	// 	- tiling mode with two windows
	// 	- client_resize the right windw
	// 	- move it to top-right
	// 	- move it to top-left
	// 	- move it bottom-left
	// 		=> clientstack_focus moves to the other window
	client_resize(c, nx, ny, geom->width, geom->height);
	statusbar_raise();
}

void action_client_move_mouse(action_arg_t const *arg){
	int x, y, ocx, ocy, nx, ny;
	client_t *c;
	xevent_t ev;
	Time lasttime = 0;


	if(!(c = dwm.focused))
		return;

	ocx = c->geom.x;
	ocy = c->geom.y;

	if(input_pointer_grab(dwm.gfx->cursors[CUR_MOVE]) != 0)
		return;

	if(!input_pointer_coord(&x, &y))
		return;

	while(xlib_get_event(&ev, true, PointerMotionMask | ButtonReleaseMask) >= 0){
		if(ev.type != MotionNotify)
			break;

		if((ev.xmotion.time - lasttime) <= (1000 / 60))
			continue;

		lasttime = ev.xmotion.time;

		nx = ocx + (ev.xmotion.x - x);
		ny = ocy + (ev.xmotion.y - y);

		if(abs(dwm.mons->x - nx) < CONFIG_SNAP_PIXEL)
			nx = dwm.mons->x;
		else if(abs((dwm.mons->x + dwm.mons->width) - (nx + WIDTH(c))) < CONFIG_SNAP_PIXEL)
			nx = dwm.mons->x + dwm.mons->width - WIDTH(c);

		if(abs(dwm.mons->y - ny) < CONFIG_SNAP_PIXEL)
			ny = dwm.mons->y;
		else if(abs((dwm.mons->y + dwm.mons->height) - (ny + HEIGHT(c))) < CONFIG_SNAP_PIXEL)
			ny = dwm.mons->y + dwm.mons->height - HEIGHT(c);

		client_resize(c, nx, ny, c->geom.width, c->geom.height);
	}

	input_pointer_release();
}

void action_client_resize(action_arg_t const *arg){
	client_t *c = dwm.focused;
	win_geom_t new;
	monitor_t *m;


	if(c == 0x0)
		return;

	m = monitor_from_client(c);
	new.width = ((int*)(arg->v))[0];
	new.height = ((int*)(arg->v))[1];

	PREP_RESIZE(x, width, &new, &c->geom, &c->geom_store, m);
	PREP_RESIZE(y, height, &new, &c->geom, &c->geom_store, m);

	client_resize(c, new.x, new.y, new.width, new.height);
	statusbar_raise();
}

void action_client_resize_mouse(action_arg_t const *arg){
	int ocx, ocy, nw, nh;
	client_t *c;
	win_geom_t *geom;
	xevent_t ev;
	Time lasttime = 0;


	if(!(c = dwm.focused))
		return;

	geom = &c->geom;
	ocx = geom->x;
	ocy = geom->y;

	if(input_pointer_grab(dwm.gfx->cursors[CUR_RESIZE]) != 0)
		return;

	input_pointer_move(c->win, geom->width + geom->border_width - 1, geom->height + geom->border_width - 1);

	while(xlib_get_event(&ev, true, PointerMotionMask | ButtonReleaseMask) >= 0){
		if(ev.type != MotionNotify)
			break;

		if((ev.xmotion.time - lasttime) <= (1000 / 60))
			continue;

		lasttime = ev.xmotion.time;

		nw = MAX(ev.xmotion.x - ocx - 2 * geom->border_width + 1, 1);
		nh = MAX(ev.xmotion.y - ocy - 2 * geom->border_width + 1, 1);

		client_resize(c, geom->x, geom->y, nw, nh);
	}

	input_pointer_move(c->win, geom->width + geom->border_width - 1, geom->height + geom->border_width - 1);
	input_pointer_release();
}


/* local functions */
static void client_cycle_complete(void){
	clientstack_focus(clientstack_cycle(0, CYCLE_END), false);
}
