#include <config/config.h>
#include <stdint.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <core/monitor.h>
#include <core/xevents.h>
#include <xlib/input.h>
#include <xlib/window.h>
#include <utils/utils.h>
#include <commands.h>


/* macros */
#define SNAP(origin, dim, new, geom, mon){ \
	if(abs((mon)->origin - *(new)) < CONFIG_SNAP_PIXEL){ \
		*new = (mon)->origin; \
	} \
	else if(abs(((mon)->origin + (mon)->dim) - (*(new) + ((geom)->dim + (geom)->border_width * 2))) < CONFIG_SNAP_PIXEL){ \
		*new = (mon)->origin + (mon)->dim - ((geom)->dim + (geom)->border_width * 2); \
	} \
}

#define MAX_TOGGLE(origin, dim, new, geom, store, mon){ \
	(new)->origin = (mon)->origin; \
	(new)->dim = (mon)->dim - (geom)->border_width * 2; \
	\
	if((geom)->origin == (new)->origin && (geom)->dim == (new)->dim){ \
		(new)->origin = (store)->origin; \
		(new)->dim = (store)->dim; \
	} \
}

#define MOVE(origin, dim, new, geom, mon) \
	switch(*new){ \
	case -INT_MAX:	*(new) = (mon)->origin; break; \
	case INT_MAX:	*(new) = (mon)->origin + (mon)->dim - (geom)->dim - (geom)->border_width * 2; break; \
	default:		*(new) += (geom)->origin; break; \
	}

#define RESIZE(origin, dim, new, hints){ \
	(new)->dim = inc_size((new)->dim, (hints)->dim ##_inc, (hints)->dim ##_base); \
	(new)->dim = MAX((new)->dim, (hints)->dim ##_min); \
}

#define RESIZE_MOVE(origin, dim, new, geom, hints){ \
	RESIZE(orgin, dim, new, hints); \
	(new)->origin = (geom)->origin - ((new)->dim - (geom)->dim) / 2; \
	\
	/* prevent moving windows if their size didn't change */ \
	if((new)->dim == (geom)->dim) \
		(new)->origin = (geom)->origin; \
}


/* local/static prototypes */
static void client_cycle_complete(void);
static int precheck(client_t *c);
static int unmax(client_t *c);
static int inc_size(int v, int inc, int base);


/* global functions */
void cmd_client_cycle(cmd_arg_t const *arg){
	client_t *c;


	c = clientstack_cycle(arg->i, keys_cycle_active() ? CYCLE_CONT : CYCLE_START);

	if(c == 0x0)
		return;

	if(!keys_cycle_active())
		keys_cycle_start(client_cycle_complete);

	clientstack_focus(c, false);

	// if the key-cycle has never been started, e.g. since the modifier keys
	// had already been released, call the cycle complete to get the same state
	// compared to a full cycle where the modifier keys are not immediately
	// released
	if(!keys_cycle_active())
		client_cycle_complete();
}

void cmd_client_kill(cmd_arg_t const *arg){
	if(dwm.focused)
		win_kill(dwm.focused->win);
}

void cmd_client_move(cmd_arg_t const *arg){
	client_t *c = dwm.focused;
	int nx,
		ny;
	win_geom_t *geom;


	if(precheck(c) != 0)
		return;

	geom = &c->geom;

	nx = ((int*)(arg->v))[0];
	ny = ((int*)(arg->v))[1];

	MOVE(x, width, &nx, geom, c->mon);
	MOVE(y, height, &ny, geom, c->mon);

	client_resize(c, nx, ny, geom->width, geom->height, geom->border_width);
}

void cmd_client_move_mouse(cmd_arg_t const *arg){
	client_t *c = dwm.focused;
	Time tlast = 0;
	int ptr_x,
		ptr_y;
	int ox,
		oy,
		nx,
		ny;
	xevent_t ev;
	win_geom_t *geom;


	if(precheck(c) != 0)
		return;

	if(input_pointer_grab(dwm.root, dwm.gfx->cursors[CUR_MOVE]) != 0)
		return;

	if(input_pointer_coord(&ptr_x, &ptr_y) != 0)
		return;

	geom = &c->geom;
	ox = geom->x;
	oy = geom->y;

	while(xlib_get_event(&ev, true, PointerMotionMask | ButtonReleaseMask) >= 0){
		if(ev.type != MotionNotify)
			break;

		if((ev.xmotion.time - tlast) <= (1000 / 60))
			continue;

		tlast = ev.xmotion.time;

		nx = ox + (ev.xmotion.x - ptr_x);
		ny = oy + (ev.xmotion.y - ptr_y);

		SNAP(x, width, &nx, geom, c->mon);
		SNAP(y, height, &ny, geom, c->mon);

		client_resize(c, nx, ny, geom->width, geom->height, geom->border_width);
	}

	input_pointer_release();
}

void cmd_client_resize(cmd_arg_t const *arg){
	client_t *c = dwm.focused;
	win_geom_t new;


	if(precheck(c) != 0)
		return;

	new.width = MAX(1, ((int*)(arg->v))[0] + c->geom.width);
	new.height = MAX(1, ((int*)(arg->v))[1] + c->geom.height);

	RESIZE_MOVE(x, width, &new, &c->geom, &c->hints);
	RESIZE_MOVE(y, height, &new, &c->geom, &c->hints);

	client_resize(c, new.x, new.y, new.width, new.height, c->geom.border_width);
}

void cmd_client_resize_mouse(cmd_arg_t const *arg){
	client_t *c = dwm.focused;
	Time tlast = 0;
	win_geom_t new;
	win_geom_t *geom;
	xevent_t ev;


	if(precheck(c) != 0)
		return;

	geom = &c->geom;

	if(input_pointer_grab(dwm.root, dwm.gfx->cursors[CUR_RESIZE]) != 0)
		return;

	input_pointer_move(c->win, geom->width + geom->border_width - 1, geom->height + geom->border_width - 1);

	while(xlib_get_event(&ev, true, PointerMotionMask | ButtonReleaseMask) >= 0){
		if(ev.type != MotionNotify)
			break;

		if((ev.xmotion.time - tlast) <= (1000 / 60))
			continue;

		tlast = ev.xmotion.time;

		new.width = ev.xmotion.x - geom->x - 2 * geom->border_width + 1;
		new.height = ev.xmotion.y - geom->y - 2 * geom->border_width + 1;

		RESIZE(x, width, &new, &c->hints);
		RESIZE(y, height, &new, &c->hints);

		client_resize(c, geom->x, geom->y, new.width, new.height, geom->border_width);
	}

	input_pointer_move(c->win, geom->width + geom->border_width - 1, geom->height + geom->border_width - 1);
	input_pointer_release();
}

void cmd_client_max(cmd_arg_t const *arg){
	client_t *c = dwm.focused;
	win_geom_t new;


	if(precheck(c) != 0)
		return;

	new = c->geom;

	if(((int*)(arg->v))[0] == 1)	MAX_TOGGLE(x, width, &new, &c->geom, &c->geom_store, c->mon);
	if(((int*)(arg->v))[1] == 1)	MAX_TOGGLE(y, height, &new, &c->geom, &c->geom_store, c->mon);

	client_resize(c, new.x, new.y, new.width, new.height, c->geom.border_width);
}

void cmd_client_fullscreen(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0 || (c->flags & WF_FULLSCREEN))
		return;

	client_flags_set(c, c->flags ^ WF_MAXED);
}


/* local functions */
static void client_cycle_complete(void){
	clientstack_focus(clientstack_cycle(0, CYCLE_END), false);
}

static int precheck(client_t *c){
	if(c == 0x0 || dwm.layout->arrange != 0x0 || unmax(c) == 0)
		return -1;

	return 0;
}

static int unmax(client_t *c){
	// fullscreen can only be disabled on client request
	if(c->flags & WF_FULLSCREEN)
		return 0;

	if((c->flags & WF_MAXED) == 0)
		return -1;

	client_flags_set(c, c->flags & ~WF_MAXED);

	return 0;
}

static int inc_size(int v, int inc, int base){
	if(inc == 0)
		return v;

	v -= base;
	v = ((v + (v > 0 ? 1 : -1) * inc / 2) / inc) * inc;
	v += base;

	return v;
}
