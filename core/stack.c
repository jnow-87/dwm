#include <config.h>
#include <core/client.h>
#include <core/dwm.h>
#include <core/scheme.h>
#include <core/stack.h>
#include <xlib/input.h>
#include <utils/list.h>
#include <utils/stack.h>


/* global functions */
client_t *client_cycle(int dir, cycle_state_t state){
	static client_t *cycle_origin = 0x0;
	client_t *c;


	switch(state){
	case CYCLE_START:
		cycle_origin = dwm.stack;

		// fall through
	case CYCLE_CONT:
		// find the next visible client, either starting at the currently focused one
		// or restart at the top of the client stack
		c = dwm.focused;
		c = (c != 0x0) ? ((dir > 0) ? c->next : c->prev) : dwm.stack;

		for(; c!=0x0; c=(dir > 0) ? c->next : c->prev){
			if(ISVISIBLE(c))
				return c;
		}

		if(dwm.focused == 0x0) // the entire stack has been checked and nothing has bee found
			return 0x0;

		// retry, this time from the top of the stack
		dwm.focused = 0x0;

		return client_cycle(dir, 0);

	case CYCLE_END:
		if(cycle_origin != 0x0){
			stack_raise(dwm.stack, cycle_origin);
			cycle_origin = 0x0;
		}

		if(dwm.focused != 0x0)
			stack_raise(dwm.stack, dwm.focused);

		return dwm.stack;
	}

	return 0x0;
}

void client_refocus(void){
	client_t *c;


	dwm.focused = 0x0;

	list_for_each(dwm.stack, c){
		if(ISVISIBLE(c) && !c->hints.never_focus)
			break;
	}

	client_focus(c, true);
}

void client_focus(client_t *c, bool restack){
	if(c == dwm.focused)
		return;

	/* unfocus client */
	if(dwm.focused){
		input_register_button_mappings(dwm.focused->win, buttons, nbuttons, 0);
		XSetWindowBorder(dwm.dpy, dwm.focused->win, dwm.scheme[SchemeNorm][ColBorder].pixel);
	}

	/* client_focus */
	dwm.focused = c;

	if(c != 0x0){
		if(restack)
			stack_raise(dwm.stack, c);

		input_register_button_mappings(c->win, buttons, nbuttons, 1);

		XSetWindowBorder(dwm.dpy, c->win, dwm.scheme[SchemeSel][ColBorder].pixel);
		XSetInputFocus(dwm.dpy, c->win, RevertToPointerRoot, CurrentTime);
		XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(c->win), 1);
		XRaiseWindow(dwm.dpy, c->win);
		client_send_event(c, dwm.wmatom[WMTakeFocus]);
	}
	else{
		XSetInputFocus(dwm.dpy, dwm.root, RevertToPointerRoot, CurrentTime);
		XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[NetActiveWindow]);
	}
}
