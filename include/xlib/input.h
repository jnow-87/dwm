#ifndef INPUT_H
#define INPUT_H


#include <stddef.h>
#include <X11/X.h>
#include <actions.h>
#include <core/buttons.h>
#include <xlib/gfx.h>
#include <xlib/window.h>


/* macros */
#define MODS_MASK	(ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)
#define LOCKS_MASK	(LockMask)


/* types */
typedef KeySym keysym_t;

typedef struct{
	click_t click;
	unsigned int mask;
	unsigned int button;

	void (*func)(action_arg_t const *arg);
	action_arg_t const arg;
} button_map_t;

typedef struct{
	unsigned int mod;
	keysym_t keysym;

	void (*func)(action_arg_t const *);
	action_arg_t const arg;
} key_map_t;


/* prototypes */
void input_keys_register(key_map_t const *mappings, size_t n);
void input_keys_release(void);
void input_buttons_register(window_t win, button_map_t const *mappings, size_t n, int focused);
void input_buttons_release(window_t win);


int input_pointer_grab(cursor_t cursor);
void input_pointer_release(void);
void input_pointer_move(window_t win, int x, int y);
int input_pointer_coord(int *x, int *y);

unsigned int input_get_numlock_mask(void);
unsigned int input_get_mod_state(void);
keysym_t input_keysym(unsigned int keycode);


#endif // INPUT_H
