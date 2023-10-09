#include <config/config.h>
#include <stdint.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <core/statusbar.h>
#include <core/xevents.h>
#include <xlib/input.h>
#include <xlib/window.h>
#include <utils/utils.h>
#include <actions.h>


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

	if(!win_send_event(dwm.focused->win, dwm.wmatom[WMDelete]))
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

	if(input_pointer_grab(dwm.gfx->cursors[CurMove]) != 0)
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
	int nx, ny, nw, nh;
	client_t *c;
	win_geom_t *geom;


	if(!(c = dwm.focused))
		return;

	geom = &c->geom;

	nw = ((int*)(arg->v))[0];
	nh = ((int*)(arg->v))[1];

	if(nw == INT_MAX){
		nx = dwm.mons->x;
		nw = dwm.mons->width - geom->border_width * 2;

		if(geom->x == nx && geom->width == nw){
			nx = c->geom_store.x;
			nw = c->geom_store.width;
		}
	}
	else{
		nx = geom->x - nw / 2;
		nw += geom->width;
	}

	if(nh == INT_MAX){
		ny = dwm.mons->y;
		nh = dwm.mons->height - geom->border_width * 2;

		if(geom->y == ny && geom->height == nh){
			ny = c->geom_store.y;
			nh = c->geom_store.height;
		}
	}
	else{
		ny = geom->y - nh / 2;
		nh += geom->height;
	}

	nw = MAX(nw, 32);
	nh = MAX(nh, 32);

	if(nw == geom->width)
		nx = geom->x;

	if(nh == geom->height)
		ny = geom->y;

	client_resize(c, nx, ny, nw, nh);
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
	ocx = c->geom.x;
	ocy = c->geom.y;

	if(input_pointer_grab(dwm.gfx->cursors[CurResize]) != 0)
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
