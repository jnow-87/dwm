#ifndef INPUT_H
#define INPUT_H


#include <stddef.h>
#include <X11/X.h>
#include <xlib/gfx.h>
#include <xlib/window.h>


/* macros */
#define MODS_MASK	(ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)
#define LOCKS_MASK	(LockMask)


/* types */
typedef KeySym keysym_t;

typedef struct{
	int start,
		end;
	int syms_per_keycode;

	KeySym *symbols;
} kbd_map_t;


/* prototypes */
int input_kbd_map_init(kbd_map_t *map);
void input_kbd_map_release(kbd_map_t *map);

void input_key_register(window_t win, keysym_t keysym, unsigned int mods, kbd_map_t *kbd_map);
void input_keys_release(window_t win);
void input_button_register(window_t win, unsigned int button, unsigned int mods);
void input_buttons_release(window_t win);

int input_pointer_grab(cursor_t cursor);
void input_pointer_release(void);
void input_pointer_move(window_t win, int x, int y);
int input_pointer_coord(int *x, int *y);

unsigned int input_get_numlock_mask(void);
unsigned int input_get_mod_state(void);
keysym_t input_keysym(unsigned int keycode);


#endif // INPUT_H
