#include <X11/X.h>
#include <config.h>
#include <core/dwm.h>
#include <utils/math.h>


/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)


/* local/static prototypes */
static void updatenumlockmask(void);


/* global functions */
int input_get_root_pointer(int *x, int *y){
	int di;
	unsigned int dui;
	Window dummy;


	return XQueryPointer(dwm.dpy, dwm.root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void input_grab_keys(void){
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

void input_grab_buttons(client_t *c, int focused){
	updatenumlockmask();
	{
		unsigned int i, j;
		unsigned int modifiers[] = {0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask};


		XUngrabButton(dwm.dpy, AnyButton, AnyModifier, c->win);

		if(!focused)
			XGrabButton(dwm.dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);

		for(i=0; i<nbuttons; i++){
			if(buttons[i].click == ClkClientWin){
				for(j=0; j<LENGTH(modifiers); j++)
					XGrabButton(dwm.dpy, buttons[i].button, buttons[i].mask | modifiers[j], c->win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
			}
		}
	}
}


/* local functions */
static void updatenumlockmask(void){
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


