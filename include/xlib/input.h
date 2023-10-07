#ifndef INPUT_H
#define INPUT_H


#include <stddef.h>
#include <X11/X.h>
#include <core/actions.h>


/* types */
typedef struct{
	unsigned int click;
	unsigned int mask;
	unsigned int button;

	void (*func)(action_arg_t const *arg);
	action_arg_t const arg;
} button_map_t;

typedef struct{
	unsigned int mod;
	KeySym keysym;

	void (*func)(action_arg_t const *);
	action_arg_t const arg;
} key_map_t;


/* prototypes */
void input_register_key_mappings(key_map_t const *mappings, size_t n);
void input_register_button_mappings(Window win, button_map_t const *mappings, size_t n, int focused);

int input_get_root_pointer(int *x, int *y);
unsigned int input_get_numlock_mask(void);


#endif // INPUT_H
