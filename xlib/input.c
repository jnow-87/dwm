#include <X11/X.h>
#include <X11/XKBlib.h>
#include <core/dwm.h>
#include <xlib/input.h>
#include <xlib/gfx.h>
#include <utils/utils.h>


/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)


/* global functions */
int input_kbd_map_init(kbd_map_t *map){
	XDisplayKeycodes(dwm.dpy, &map->start, &map->end);
	map->symbols = XGetKeyboardMapping(dwm.dpy, map->start, map->end - map->start + 1, &map->syms_per_keycode);

	if(map->symbols == 0x0)
		return -1;

	return 0;
}

void input_kbd_map_release(kbd_map_t *map){
	XFree(map->symbols);
}

void input_key_register(window_t win, keysym_t keysym, unsigned int mods, kbd_map_t *kbd_map){
	unsigned int modifiers[] = { 0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask };


	for(int i=kbd_map->start; i<=kbd_map->end; i++){
		/* skip modifier codes, we do that ourselves */
		if(keysym == kbd_map->symbols[(i - kbd_map->start) * kbd_map->syms_per_keycode]){
			for(int j=0; j<LENGTH(modifiers); j++)
				XGrabKey(dwm.dpy, i, mods | modifiers[j], win, True, GrabModeAsync, GrabModeAsync);
		}
	}
}

void input_keys_release(window_t win){
	XUngrabKey(dwm.dpy, AnyKey, AnyModifier, win);
}

void input_button_register(window_t win, unsigned int button, unsigned int mods){
	unsigned int modifiers[] = {0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask};


	for(size_t i=0; i<LENGTH(modifiers); i++)
		XGrabButton(dwm.dpy, button, mods | modifiers[i], win, False, BUTTONMASK, GrabModeAsync, GrabModeSync, None, None);
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
