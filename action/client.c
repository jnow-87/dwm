#include <config/config.h>
#include <stdint.h>
#include <X11/X.h>
#include <utils/math.h>
#include <core/dwm.h>
#include <core/actions.h>
#include <core/clientstack.h>
#include <xlib/window.h>
#include <xlib/input.h>
#include <core/xevents.h>
#include <core/statusbar.h>
#include <core/keys.h>


/* macros */
#define MOUSEMASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask)


/* local/static prototypes */
static void focusstack_complete(void);


/* global functions */
void action_focusstack(action_arg_t const *arg){
	client_t *c;


	c = clientstack_cycle(arg->i, keys_cycle_active() ? CYCLE_CONT : CYCLE_START);

	if(c == 0x0)
		return;

	if(!keys_cycle_active())
		keys_cycle_start(focusstack_complete);

	clientstack_focus(c, false);

	statusbar_raise();
}

void action_killclient(action_arg_t const *arg){
	if(!dwm.focused)
		return;

	if(!win_send_event(dwm.focused->win, dwm.wmatom[WMDelete]))
		win_kill(dwm.focused->win);
}

void action_movemouse(action_arg_t const *arg){
	int x, y, ocx, ocy, nx, ny;
	client_t *c;
	XEvent ev;
	Time lasttime = 0;


	if(!(c = dwm.focused))
		return;

	ocx = c->geom.x;
	ocy = c->geom.y;

	if(XGrabPointer(dwm.dpy, dwm.root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, dwm.cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	if(!input_get_root_pointer(&x, &y))
		return;

	do{
		XMaskEvent(dwm.dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch(ev.type){
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			xevents_handle_event(&ev);
			break;

		case MotionNotify:
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
			break;
		}
	} while(ev.type != ButtonRelease);
	
	XUngrabPointer(dwm.dpy, CurrentTime);
}

void action_moveclient(action_arg_t const *arg){
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

void action_reszclient(action_arg_t const *arg){
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

void action_resizemouse(action_arg_t const *arg){
	int ocx, ocy, nw, nh;
	client_t *c;
	win_geom_t *geom;
	XEvent ev;
	Time lasttime = 0;


	if(!(c = dwm.focused))
		return;

	geom = &c->geom;
	ocx = c->geom.x;
	ocy = c->geom.y;

	if(XGrabPointer(dwm.dpy, dwm.root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, dwm.cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;

	XWarpPointer(dwm.dpy, None, c->win, 0, 0, 0, 0, geom->width + geom->border_width - 1, geom->height + geom->border_width - 1);

	do{
		XMaskEvent(dwm.dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch(ev.type){
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			xevents_handle_event(&ev);
			break;

		case MotionNotify:
			if((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;

			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * geom->border_width + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * geom->border_width + 1, 1);

			client_resize(c, geom->x, geom->y, nw, nh);

			break;
		}
	} while(ev.type != ButtonRelease);

	XWarpPointer(dwm.dpy, None, c->win, 0, 0, 0, 0, geom->width + geom->border_width - 1, geom->height + geom->border_width - 1);
	XUngrabPointer(dwm.dpy, CurrentTime);

	while(XCheckMaskEvent(dwm.dpy, EnterWindowMask, &ev));
}


/* local functions */
static void focusstack_complete(void){
	client_t *c;


	c = clientstack_cycle(0, CYCLE_END);

	clientstack_focus(c, false);
}
