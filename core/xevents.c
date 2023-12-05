#include <stdbool.h>
#include <X11/Xlib.h>
#include <core/buttons.h>
#include <core/clientstack.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <core/layout.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/xevents.h>
#include <xlib/input.h>
#include <xlib/window.h>
#include <xlib/xlib.h>


/* local/static prototypes */
static void destroy_notify(xevent_t *e);

static void client_message(xevent_t *e);
static void configure_request(xevent_t *e);
static void configure_notify(xevent_t *e);
static void property_notify(xevent_t *e);

static void map_request(xevent_t *e);
static void mapping_notify(xevent_t *e);
static void unmap_notify(xevent_t *e);

static void expose(xevent_t *e);
static void focus_in(xevent_t *e);

static void key_press(xevent_t *e);
static void button_press(xevent_t *e);


/* static variables */
static void (*handler[LASTEvent])(xevent_t*) = {
	[DestroyNotify] = destroy_notify,
	[ClientMessage] = client_message,
	[ConfigureRequest] = configure_request,
	[ConfigureNotify] = configure_notify,
	[PropertyNotify] = property_notify,
	[EnterNotify] = 0x0,
	[MapRequest] = map_request,
	[MappingNotify] = mapping_notify,
	[UnmapNotify] = unmap_notify,
	[MotionNotify] = 0x0,
	[Expose] = expose,
	[FocusIn] = focus_in,
	[KeyPress] = key_press,
	[ButtonPress] = button_press,
};


/* global functions */
int xevents_handle_events(void){
	int r = 1;
	xevent_t ev;


	while(r > 0){
		r = xlib_get_event(&ev, false, 0);

		if(r > 0)
			xevents_handle_event(&ev);
	}

	return r;
}

void xevents_handle_event(xevent_t *ev){
	if(handler[ev->type] != 0x0)
		handler[ev->type](ev);
}


/* local functions */
static void destroy_notify(xevent_t *e){
	XDestroyWindowEvent *ev = &e->xdestroywindow;
	client_t *c;


	c = client_from_win(ev->window);

	if(c == 0x0)
		return;

	client_cleanup(c, true);
	layout_arrange();
}

static void client_message(xevent_t *e){
	XClientMessageEvent *ev = &e->xclient;
	win_flags_t fullscreen;
	client_t *c;


	c = client_from_win(ev->window);

	if(c == 0x0 || ev->message_type != netatom_get(NET_WM_STATE))
		return;

	if(ev->data.l[1] == netatom_get(NET_WM_FULLSCREEN) || ev->data.l[2] == netatom_get(NET_WM_FULLSCREEN)){
		fullscreen = (ev->data.l[0] == 1 || ev->data.l[0] == 2) ? WF_FULLSCREEN : 0;
		client_flags_set(c, (c->flags & ~WF_FULLSCREEN) | fullscreen);
	}
}

static void configure_request(xevent_t *e){
	XConfigureRequestEvent *ev = &e->xconfigurerequest;
	client_t *c;
	win_geom_t *geom;
	XWindowChanges wc;


	c = client_from_win(ev->window);

	if(c != 0x0){
		geom = &c->geom;
		c->geom_store = c->geom;

		if(ev->value_mask & CWBorderWidth)	geom->border_width = ev->border_width;
		if(ev->value_mask & CWX)			geom->x = ev->x;
		if(ev->value_mask & CWY)			geom->y = ev->y;
		if(ev->value_mask & CWWidth)		geom->width = ev->width;
		if(ev->value_mask & CWHeight)		geom->height = ev->height;

		win_resize(c->win, geom, &c->hints);
	}
	else{
		wc.x = ev->x;
		wc.y = ev->y;
		wc.width = ev->width;
		wc.height = ev->height;
		wc.border_width = ev->border_width;
		wc.sibling = ev->above;
		wc.stack_mode = ev->detail;

		XConfigureWindow(dwm.dpy, ev->window, ev->value_mask, &wc);
	}

	xlib_sync();
}

static void configure_notify(xevent_t *e){
	XConfigureEvent *ev = &e->xconfigure;


	if(ev->window != dwm.root)
		return;

	dwm.screen_width = ev->width;
	dwm.screen_height = ev->height;

	monitor_discover();

	gfx_resize(dwm.gfx, dwm.screen_width, dwm.screen_height);
	layout_arrange();
}

static void property_notify(xevent_t *e){
	XPropertyEvent *ev = &e->xproperty;
	client_t *c;


	if(ev->state == PropertyDelete)
		return;

	if((ev->window == dwm.root) && (ev->atom == XA_WM_NAME)){
		statusbar_update();
	}
	else if((c = client_from_win(ev->window))){
		switch(ev->atom){
		case XA_WM_NORMAL_HINTS:	win_update_sizehints(c->win, &c->hints); break;
		case XA_WM_HINTS:			win_update_wmhints(c->win, &c->hints, c == dwm.focused); break;
		}
	}
}

static void map_request(xevent_t *e){
	XMapRequestEvent *ev = &e->xmaprequest;
	win_attr_t attr;
	client_t *c;


	c = client_from_win(ev->window);

	if(c == 0x0){
		if(win_get_attr(ev->window, &attr) != 0 || attr.override_redirect)
			return;

		client_init(ev->window, &attr);
		layout_arrange();
	}
	else
		win_show(c->win);
}

static void mapping_notify(xevent_t *e){
	XMappingEvent *ev = &e->xmapping;


	XRefreshKeyboardMapping(ev);

	if(ev->request == MappingKeyboard)
		keys_register();
}

static void unmap_notify(xevent_t *e){
	XUnmapEvent *ev = &e->xunmap;
	client_t *c;


	c = client_from_win(ev->window);

	if(c != 0x0 && ev->send_event)
			win_set_state(c->win, WithdrawnState);
}

static void expose(xevent_t *e){
	XExposeEvent *ev = &e->xexpose;


	if(ev->window == dwm.statusbar.win && ev->count == 0)
		statusbar_update();
}

static void focus_in(xevent_t *e){
	XFocusChangeEvent *ev = &e->xfocus;


	if(dwm.focused && ev->window != dwm.focused->win)
		clientstack_focus(dwm.focused, false);
}

static void key_press(xevent_t *e){
	XKeyEvent *ev = &e->xkey;


	keys_handle(input_keysym(ev->keycode), ev->state);
}

static void button_press(xevent_t *e){
	button_loc_t loc = BLOC_ROOT;
	XButtonPressedEvent *ev = &e->xbutton;
	client_t *c;


	if(ev->window != dwm.statusbar.win){
		c = client_from_win(ev->window);

		if(c != 0x0){
			loc = BLOC_CLIENT;

			clientstack_focus(c, true);
			xlib_release_events();
		}
	}
	else
		loc = statusbar_element(ev->x, ev->y);

	button_handle(loc, ev->button, ev->state);
}
