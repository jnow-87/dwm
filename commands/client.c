#include <config/config.h>
#include <stdint.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <core/monitor.h>
#include <core/xevents.h>
#include <utils/log.h>
#include <utils/math.h>
#include <xlib/input.h>
#include <xlib/window.h>
#include <commands.h>
#include <rc.h>


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


/* types */
typedef enum{
	ARG_CYCLE_FWD = 0x1,
	ARG_CYCLE_IGNORE_ZAPHOD = 0x2,
} cycle_arg_t;

typedef enum{
	ARG_MV_LEFT = 0x1,
	ARG_MV_RIGHT = 0x2,
	ARG_MV_UP = 0x4,
	ARG_MV_DOWN = 0x8,
	ARG_MV_BORDER = 0x10,
} move_arg_t;

typedef enum{
	ARG_RSZ_SHRINK = 0x1,
	ARG_RSZ_VERT = 0x2,
	ARG_RSZ_HOR = 0x4,
} resize_arg_t;


/* local/static prototypes */
static void client_cycle_complete(void);
static int precheck(client_t *c);
static int unmax(client_t *c);
static int inc_size(int v, int inc, int base);


/* global functions */
void cmd_client_cycle(cmd_arg_t const *arg){
	int dir = arg->ui & ARG_CYCLE_FWD;
	bool ignore_zaphod = (bool)(arg->ui & ARG_CYCLE_IGNORE_ZAPHOD);
	client_t *c;


	c = clientstack_cycle(dir, keys_cycle_active() ? CYCLE_CONT : CYCLE_START, ignore_zaphod);

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

int cmd_client_cycle_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg){
	switch(tk_id){
	case ARG_FORWARD:		arg->ui |= ARG_CYCLE_FWD; break;
	case ARG_BACKWARD:		arg->ui &= ~ARG_CYCLE_FWD; break;
	case ARG_IGNORE_ZAPHOD:	arg->ui |= ARG_CYCLE_IGNORE_ZAPHOD; break;
	case ARG_ZAPHOD:		arg->ui &= ~ARG_CYCLE_IGNORE_ZAPHOD; break;
	default:				return ERROR("invalid client cycle argument: %s\n", tk);
	}

	return 0;
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

	ny = (arg->ui & ARG_MV_BORDER) ? INT_MAX : rc.win_move_pixel;
	nx = ny * ((bool)(arg->ui & ARG_MV_RIGHT)) - ny * ((bool)(arg->ui & ARG_MV_LEFT));
	ny = ny * ((bool)(arg->ui & ARG_MV_DOWN)) - ny * ((bool)(arg->ui & ARG_MV_UP));

	MOVE(x, width, &nx, geom, c->mon);
	MOVE(y, height, &ny, geom, c->mon);

	client_resize(c, nx, ny, geom->width, geom->height, geom->border_width);
}

int cmd_client_move_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg){
	switch(tk_id){
	case ARG_LEFT:		arg->ui |= ARG_MV_LEFT; break;
	case ARG_RIGHT:		arg->ui |= ARG_MV_RIGHT; break;
	case ARG_UP:		// fall through
	case ARG_TOP:		arg->ui |= ARG_MV_UP; break;
	case ARG_DOWN:		// fall through
	case ARG_BOTTOM:	arg->ui |= ARG_MV_DOWN; break;
	case ARG_BORDER:	arg->ui |= ARG_MV_BORDER; break;
	default:			return ERROR("invalid client move argument: %s\n", tk);
	}

	return 0;
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
	int dx,
		dy;
	win_geom_t new;


	if(precheck(c) != 0)
		return;

	dy = rc.win_resize_pixel;
	dy = dy - 2 * dy * ((bool)(arg->ui & ARG_RSZ_SHRINK));
	dx = dy * ((bool)(arg->ui & ARG_RSZ_HOR));
	dy = dy * ((bool)(arg->ui & ARG_RSZ_VERT));

	new.width = MAX(1, c->geom.width + dx);
	new.height = MAX(1, c->geom.height + dy);

	RESIZE_MOVE(x, width, &new, &c->geom, &c->hints);
	RESIZE_MOVE(y, height, &new, &c->geom, &c->hints);

	client_resize(c, new.x, new.y, new.width, new.height, c->geom.border_width);
}

int cmd_client_resize_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg){
	switch(tk_id){
	case ARG_SHRINK:	arg->ui |= ARG_RSZ_SHRINK; break;
	case ARG_GROW:		arg->ui &= ~ARG_RSZ_SHRINK; break;
	case ARG_VERT:		arg->ui |= ARG_RSZ_VERT; break;
	case ARG_HOR:		arg->ui |= ARG_RSZ_HOR; break;
	default:			return ERROR("invalid client resize argument: %s\n", tk);
	}

	return 0;
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

	if(arg->ui & ARG_RSZ_HOR)	MAX_TOGGLE(x, width, &new, &c->geom, &c->geom_store, c->mon);
	if(arg->ui & ARG_RSZ_VERT)	MAX_TOGGLE(y, height, &new, &c->geom, &c->geom_store, c->mon);

	client_resize(c, new.x, new.y, new.width, new.height, c->geom.border_width);
}

int cmd_client_max_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg){
	switch(tk_id){
	case ARG_VERT:		arg->ui |= ARG_RSZ_VERT; break;
	case ARG_HOR:		arg->ui |= ARG_RSZ_HOR; break;
	default:			return ERROR("invalid client max argument: %s\n", tk);
	}

	return 0;
}

void cmd_client_fullscreen(cmd_arg_t const *arg){
	client_t *c = dwm.focused;


	if(c == 0x0 || (c->flags & WF_FULLSCREEN))
		return;

	client_flags_set(c, c->flags ^ WF_MAXED);
}


/* local functions */
static void client_cycle_complete(void){
	clientstack_focus(clientstack_cycle(0, CYCLE_END, false), false);
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
