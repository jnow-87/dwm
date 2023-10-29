#include <stdbool.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <core/dwm.h>
#include <xlib/input.h>
#include <xlib/gfx.h>
#include <xlib/window.h>
#include <xlib/xlib.h>
#include <utils/utils.h>


/* local/static prototypes */
static void set_border(window_t win, scheme_id_t scheme);
static void apply_sizehints(window_t win, win_geom_t *geom, win_hints_t *hints);
static int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee);


/* global functions */
window_t win_create(win_geom_t *geom, cursor_type_t cursor, char *class, bool override_redirect){
	window_t win;


	win = XCreateWindow(
		dwm.dpy,
		dwm.root,
		geom->x,
		geom->y,
		geom->width,
		geom->height,
		0,
		DefaultDepth(dwm.dpy, dwm.screen),
		CopyFromParent,
		DefaultVisual(dwm.dpy, dwm.screen),
		CWOverrideRedirect | CWBackPixmap | CWEventMask,
		&(XSetWindowAttributes){
			.override_redirect = override_redirect ? True : False,
			.background_pixmap = ParentRelative,
			.event_mask = ButtonPressMask | ExposureMask
		}
	);

	if(cursor != CUR_NONE)
		XDefineCursor(dwm.dpy, win, dwm.gfx->cursors[cursor]);

	if(class != 0x0)
		XSetClassHint(dwm.dpy, win, &(XClassHint){class, class});

	XMapWindow(dwm.dpy, win);

	return win;
}

void win_destroy(window_t win){
	XUnmapWindow(dwm.dpy, win);
	XDestroyWindow(dwm.dpy, win);
}

void win_init(window_t win, win_geom_t *geom, win_hints_t *hints){
	win_configure(win, geom);

	XConfigureWindow(dwm.dpy, win, CWBorderWidth, &((XWindowChanges){ .border_width = geom->border_width }));
	XSelectInput(dwm.dpy, win, FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
	XMoveResizeWindow(dwm.dpy, win, geom->x + 2 * dwm.screen_width, geom->y, geom->width, geom->height); //some windows require this

	set_border(win, SCM_NORM);
	win_update_sizehints(win, hints);
	win_update_wmhints(win, hints, false);
	win_set_state(win, NormalState);
	win_set_flags(win, 0);
	
	XMapWindow(dwm.dpy, win);
}

void win_kill(window_t win){
	XGrabServer(dwm.dpy);
	xlib_set_error_handler(dummy_xerror_hdlr);
	XSetCloseDownMode(dwm.dpy, DestroyAll);

	XKillClient(dwm.dpy, win);
	xlib_sync();

	xlib_set_error_handler(0x0);
	XUngrabServer(dwm.dpy);
}

void win_release(window_t win, win_geom_t *original){
	XWindowChanges wc;


	wc.border_width = original->border_width;

	XGrabServer(dwm.dpy); /* avoid race conditions */
	xlib_set_error_handler(dummy_xerror_hdlr);

	XSelectInput(dwm.dpy, win, NoEventMask);
	XConfigureWindow(dwm.dpy, win, CWBorderWidth, &wc); /* restore border */
	input_buttons_release(win);
	win_set_state(win, WithdrawnState);
	xlib_sync();

	xlib_set_error_handler(0x0);
	XUngrabServer(dwm.dpy);
}

void win_configure(window_t win, win_geom_t *geom){
	XConfigureEvent ce;


	ce.type = ConfigureNotify;
	ce.display = dwm.dpy;
	ce.event = win;
	ce.window = win;
	ce.x = geom->x;
	ce.y = geom->y;
	ce.width = geom->width;
	ce.height = geom->height;
	ce.border_width = geom->border_width;
	ce.above = None;
	ce.override_redirect = False;

	XSendEvent(dwm.dpy, win, False, StructureNotifyMask, (XEvent*)&ce);
}

void win_resize(window_t win, win_geom_t *geom, win_hints_t *hints){
	XWindowChanges wc;


	if(hints != 0x0)
		apply_sizehints(win, geom, hints);

	wc.x = geom->x;
	wc.y = geom->y;
	wc.width = geom->width;
	wc.height = geom->height;
	wc.border_width = geom->border_width;

	XConfigureWindow(dwm.dpy, win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
	win_configure(win, geom);
	xlib_sync();
}

void win_raise(window_t win){
	XRaiseWindow(dwm.dpy, win);
}

void win_set_state(window_t win, long state){
	long data[] = { state, None };


	XChangeProperty(dwm.dpy, win, dwm.wmatom[WM_STATE], dwm.wmatom[WM_STATE], 32, PropModeReplace, (unsigned char *)data, 2);
}

void win_set_flags(window_t win, unsigned int mask){
	XChangeProperty(dwm.dpy, win, dwm.netatom[NET_WM_STATE], XA_ATOM, 32, PropModeReplace, 0x0, 0);

	if(mask & WF_MAXED){
		XChangeProperty(dwm.dpy, win, dwm.netatom[NET_WM_STATE], XA_ATOM, 32, PropModeAppend, (unsigned char*)&dwm.netatom[NET_WM_VERTMAX], 1);
		XChangeProperty(dwm.dpy, win, dwm.netatom[NET_WM_STATE], XA_ATOM, 32, PropModeAppend, (unsigned char*)&dwm.netatom[NET_WM_HORMAX], 1);
	}

	if(mask & WF_FULLSCREEN)
		XChangeProperty(dwm.dpy, win, dwm.netatom[NET_WM_STATE], XA_ATOM, 32, PropModeAppend, (unsigned char*)&dwm.netatom[NET_WM_FULLSCREEN], 1);
}

bool win_send_event(window_t win, Atom proto){
	bool exists = false;
	int n;
	Atom *protocols;
	XEvent ev;


	if(XGetWMProtocols(dwm.dpy, win, &protocols, &n)){
		while(!exists && n--)
			exists = (protocols[n] == proto);

		XFree(protocols);
	}

	if(!exists)
		return false;

	ev.type = ClientMessage;
	ev.xclient.window = win;
	ev.xclient.message_type = dwm.wmatom[WM_PROTOCOLS];
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = proto;
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(dwm.dpy, win, False, NoEventMask, &ev);

	return true;
}

void win_show(window_t win, win_geom_t *geom){
	XMoveWindow(dwm.dpy, win, geom->x, geom->y);
}

void win_hide(window_t win, win_geom_t *geom){
	XMoveWindow(dwm.dpy, win, geom->width * -2, geom->y);
}

bool win_visible(window_t win, win_geom_t *geom){
	win_attr_t attr;


	if(win_get_attr(win, &attr) != 0)
		return false;

	return (attr.geom.x == geom->x && attr.geom.y == geom->y);
}

void win_focus(window_t win){
	XSetInputFocus(dwm.dpy, win, RevertToPointerRoot, CurrentTime);

	if(win != dwm.root){
		set_border(win, SCM_FOCUS);

		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NET_ACTIVE_WINDOW], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(win), 1);
		XRaiseWindow(dwm.dpy, win);
		win_send_event(win, dwm.wmatom[WM_TAKEFOCUS]);
	}
	else
		XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NET_ACTIVE_WINDOW]);
}

void win_unfocus(window_t win){
	set_border(win, SCM_NORM);
}

int win_get_attr(window_t win, win_attr_t *attr){
	XWindowAttributes wa;


	if(!XGetWindowAttributes(dwm.dpy, win, &wa))
		return -1;

	attr->map_state = wa.map_state;
	attr->override_redirect = !!wa.override_redirect;
	attr->geom.x = wa.x;
	attr->geom.y = wa.y;
	attr->geom.width = wa.width;
	attr->geom.height = wa.height;
	attr->geom.border_width = wa.border_width;

	return 0;
}

long win_get_state(window_t win){
	int format;
	long result = -1;
	unsigned char *p = 0x0;
	unsigned long n, extra;
	Atom real;


	if(XGetWindowProperty(dwm.dpy, win, dwm.wmatom[WM_STATE], 0L, 2L, False, dwm.wmatom[WM_STATE], &real, &format, &n, &extra, (unsigned char **)&p) != Success)
		return -1;

	if(n != 0)
		result = *p;

	XFree(p);

	return result;
}

window_t win_get_transient(window_t win){
	window_t trans;


	if(XGetTransientForHint(dwm.dpy, win, &trans))
		return trans;

	return None;
}

void win_update_wmhints(window_t win, win_hints_t *hints, bool isfocused){
	XWMHints *wmh;


	if((wmh = XGetWMHints(dwm.dpy, win))){
		if(isfocused && wmh->flags & XUrgencyHint){
			wmh->flags &= ~XUrgencyHint;
			XSetWMHints(dwm.dpy, win, wmh);
		}

		if(wmh->flags & InputHint)	hints->never_focus = !wmh->input;
		else						hints->never_focus = 0;

		XFree(wmh);
	}
}

void win_update_sizehints(window_t win, win_hints_t *hints){
	long msize;
	XSizeHints size;


	if(!XGetWMNormalHints(dwm.dpy, win, &size, &msize)){
		/* size is uninitialized, ensure that size.flags aren't used */
		size.flags = PSize;
	}

	if(size.flags & PBaseSize){
		hints->width_base = size.base_width;
		hints->height_base = size.base_height;
	}
	else if(size.flags & PMinSize){
		hints->width_base = size.min_width;
		hints->height_base = size.min_height;
	}
	else
		hints->width_base = hints->height_base = 0;

	if(size.flags & PResizeInc){
		hints->width_inc = size.width_inc;
		hints->height_inc = size.height_inc;
	}
	else
		hints->width_inc = hints->height_inc = 0;

	if(size.flags & PMaxSize){
		hints->width_max = size.max_width;
		hints->height_max = size.max_height;
	}
	else
		hints->width_max = hints->height_max = 0;

	if(size.flags & PMinSize){
		hints->width_min = size.min_width;
		hints->height_min = size.min_height;
	}
	else if(size.flags & PBaseSize){
		hints->width_min = size.base_width;
		hints->height_min = size.base_height;
	}
	else
		hints->width_min = hints->height_min = 0;

	if(size.flags & PAspect){
		hints->aspect_min = (float)size.min_aspect.y / size.min_aspect.x;
		hints->aspect_max = (float)size.max_aspect.x / size.max_aspect.y;
	}
	else
		hints->aspect_max = hints->aspect_min = 0.0;
}


/* local functions */
static void set_border(window_t win, scheme_id_t scheme){
	XSetWindowBorder(dwm.dpy, win, dwm.gfx->schemes[scheme].border.pixel);
}

static void apply_sizehints(window_t win, win_geom_t *geom, win_hints_t *hints){
	int baseismin;


	/* set minimum possible */
	geom->width = MAX(1, geom->width);
	geom->height = MAX(1, geom->height);

	/* see last two sentences in ICCCM 4.1.2.3 */
	baseismin = ((hints->width_base == hints->width_min) && (hints->height_base == hints->height_min));

	if(!baseismin){ /* temporarily remove base dimensions */
		geom->width -= hints->width_base;
		geom->height -= hints->height_base;
	}

	/* adjust for aspect limits */
	if(hints->aspect_min > 0 && hints->aspect_max > 0){
		if(hints->aspect_max < (float)geom->width / geom->height)		geom->width = geom->height * hints->aspect_max + 0.5;
		else if(hints->aspect_min < (float)geom->height / geom->width)	geom->height = geom->width * hints->aspect_min + 0.5;
	}

	if(baseismin){ /* increment calculation requires this */
		geom->width -= hints->width_base;
		geom->height -= hints->height_base;
	}

	/* adjust for increment value */
	if(hints->width_inc)
		geom->width -= geom->width % hints->width_inc;

	if(hints->height_inc)
		geom->height -= geom->height % hints->height_inc;

	/* restore base dimensions */
	geom->width = MAX(geom->width + hints->width_base, hints->width_min);
	geom->height = MAX(geom->height + hints->height_base, hints->height_min);

	if(hints->width_max)
		geom->width = MIN(geom->width, hints->width_max);

	if(hints->height_max)
		geom->height = MIN(geom->height, hints->height_max);
}

static int dummy_xerror_hdlr(Display *dpy, XErrorEvent *ee){
	return 0;
}
