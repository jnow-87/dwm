#include <X11/X.h>
#include <X11/XKBlib.h>
#include <core/buttons.h>
#include <core/dwm.h>
#include <xlib/input.h>
#include <xlib/gfx.h>
#include <utils/utils.h>


/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)


/* global functions */
void input_keys_register(key_map_t const *mappings, size_t n){
	unsigned int modifiers[] = { 0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask };
	int start,
		end,
		syms_per_keycode;
	KeySym *syms;


	input_keys_release();
	XDisplayKeycodes(dwm.dpy, &start, &end);
	syms = XGetKeyboardMapping(dwm.dpy, start, end - start + 1, &syms_per_keycode);

	if(!syms)
		return;

	for(int k=start; k<=end; k++){
		for(size_t i=0; i<n; i++){
			/* skip modifier codes, we do that ourselves */
			if(mappings[i].keysym == syms[(k - start) * syms_per_keycode]){
				for(int j=0; j<LENGTH(modifiers); j++)
					XGrabKey(dwm.dpy, k, mappings[i].mod | modifiers[j], dwm.root, True, GrabModeAsync, GrabModeAsync);
			}
		}
	}

	XFree(syms);
}

void input_keys_release(void){
	XUngrabKey(dwm.dpy, AnyKey, AnyModifier, dwm.root);
}

void input_buttons_register(Window win, button_map_t const *mappings, size_t n, int focused){
	unsigned int modifiers[] = {0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask};


	input_buttons_release(win);

	if(!focused)
		XGrabButton(dwm.dpy, AnyButton, AnyModifier, win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);

	for(size_t i=0; i<n; i++){
		if(mappings[i].click == CLK_CLIENT){
			for(size_t j=0; j<LENGTH(modifiers); j++)
				XGrabButton(dwm.dpy, mappings[i].button, mappings[i].mask | modifiers[j], win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
		}
	}
}

void input_buttons_release(window_t win){
	XUngrabButton(dwm.dpy, AnyButton, AnyModifier, win);
}

int input_pointer_grab(cursor_t cursor){
	return -(XGrabPointer(dwm.dpy, dwm.root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime) != GrabSuccess);
}

void input_pointer_release(void){
	XEvent ev;


	XUngrabPointer(dwm.dpy, CurrentTime);
	while(XCheckMaskEvent(dwm.dpy, EnterWindowMask, &ev));
}

void input_pointer_move(window_t win, int x, int y){
	XWarpPointer(dwm.dpy, None, win, 0, 0, 0, 0, x, y);
}

int input_pointer_coord(int *x, int *y){
	int di;
	unsigned int dui;
	Window dummy;


	return XQueryPointer(dwm.dpy, dwm.root, &dummy, &dummy, x, y, &di, &di, &dui);
}

unsigned int input_get_numlock_mask(void){
	unsigned int mask = 0;
	XModifierKeymap *modmap;


	modmap = XGetModifierMapping(dwm.dpy);

	for(int i=0; i<8; i++){
		for(int j=0; j<modmap->max_keypermod; j++){
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dwm.dpy, XK_Num_Lock)){
				mask = (1 << i);
				break;
			}
		}
	}

	XFreeModifiermap(modmap);

	return mask;
}

unsigned int input_get_mod_state(void){
	XkbStateRec state;


	XkbGetState(dwm.dpy, XkbUseCoreKbd, &state);

	return state.mods;
}

keysym_t input_keysym(unsigned int keycode){
	return XKeycodeToKeysym(dwm.dpy, (KeyCode)keycode, 0);
}
