/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the client_focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys mappings and other configurations are defined in config.h.
 *
 * To understand everything else, start reading main().
 */

#include <X11/X.h>
#include <X11/Xatom.h>
#include <config/config.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <xlib/client.h>
#include <core/scheme.h>
#include <config.h>
#include <core/dwm.h>
#include <xlib/monitor.h>
#include <core/statusbar.h>
#include <utils/math.h>
#include <xlib/xinerama.h>


/* global functions */
void die(char const *fmt, ...){
	va_list ap;


	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if(fmt[0] && fmt[strlen(fmt) - 1] == ':'){
		fputc(' ', stderr);
		perror(NULL);
	}
	else
		fputc('\n', stderr);

	exit(1);
}

int getrootptr(int *x, int *y){
	int di;
	unsigned int dui;
	Window dummy;


	return XQueryPointer(dwm.dpy, dwm.root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void grabkeys(void){
	updatenumlockmask();
	{
		unsigned int modifiers[] = { 0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask };
		int start, end, syms_per_keycode;
		KeySym *syms;


		XUngrabKey(dwm.dpy, AnyKey, AnyModifier, dwm.root);
		XDisplayKeycodes(dwm.dpy, &start, &end);
		syms = XGetKeyboardMapping(dwm.dpy, start, end - start + 1, &syms_per_keycode);

		if(!syms)
			return;

		for(int k=start; k<=end; k++){
			for(int i=0; i<nkeys; i++){
				/* skip modifier codes, we do that ourselves */
				if(keys[i].keysym == syms[(k - start) * syms_per_keycode]){
					for(int j=0; j<LENGTH(modifiers); j++)
						XGrabKey(dwm.dpy, k, keys[i].mod | modifiers[j], dwm.root, True, GrabModeAsync, GrabModeAsync);
				}
			}
		}

		XFree(syms);
	}
}

void monitor_discover(void){
	/* free existing monitors */
	while(dwm.mons){
		monitor_destroy(dwm.mons);
	}

	if(xinerama_discover() < 0)
		monitor_create(0, 0, dwm.screen_width, dwm.screen_height);
}

void updatenumlockmask(void){
	unsigned int i, j;
	XModifierKeymap *modmap;


	dwm.numlock_mask = 0;
	modmap = XGetModifierMapping(dwm.dpy);

	for(i=0; i<8; i++){
		for(j=0; j<modmap->max_keypermod; j++){
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dwm.dpy, XK_Num_Lock))
				dwm.numlock_mask = (1 << i);
		}
	}

	XFreeModifiermap(modmap);
}

int event_add(int fd, event_hdlr_t hdlr){
	struct epoll_event ev;


	ev.events = EPOLLIN;
	ev.data.ptr = hdlr;

	return epoll_ctl(dwm.event_fd, EPOLL_CTL_ADD, fd, &ev);
}
