#include <core/dwm.h>
#include <core/keys.h>
#include <xlib/input.h>
#include <utils/timer.h>
#include <utils/log.h>
#include <utils/utils.h>


/* local/static prototypes */
static int modifier_reset_hdlr(void);


/* static variables */
static unsigned int modifier_state = 0;
static int modifier_reset_timer = -1;
static cycle_callback_t cycle_complete = 0x0;


/* global functions */
int keys_init(void){
	int r = 0;


	modifier_reset_timer = timer_init();

	if(modifier_reset_timer == -1)
		return -1;

	r |= keys_register();
	r |= dwm_hdlr_add(modifier_reset_timer, modifier_reset_hdlr);

	if(r == 0)
		return 0;

	keys_cleanup();

	return -1;
}

void keys_cleanup(void){
	input_keys_release(dwm.root);
	timer_destroy(modifier_reset_timer);
}

int keys_register(void){
	kbd_map_t map;
	keymap_t *key;


	input_keys_release(dwm.root);

	if(input_kbd_map_init(&map) != 0)
		return -1;

	config_for_each(keys, key)
		input_key_register(dwm.root, key->keysym, key->mods, &map);

	input_kbd_map_release(&map);

	return 0;
}

void keys_handle(keysym_t sym, unsigned int mods){
	keymap_t *key;


	config_for_each(keys, key){
		if(keys_registered(key, sym, mods))
			key->action(&(key->arg));
	}
}

bool keys_registered(keymap_t *key, keysym_t sym, unsigned int mods){
	return (sym == key->keysym) && (CLEANMODS(key->mods) == CLEANMODS(mods)) && (key->action != 0x0);
}

void keys_cycle_start(cycle_callback_t complete){
	unsigned int mods;


	keys_cycle_complete();

	mods = input_get_mod_state();

	if(mods == 0)
		return;

	// the xlib KeyRelease event does not reliably report the release of modifier keys,
	// hence use a timer to reset the modifier state manually
	if(timer_set(modifier_reset_timer, 100) != 0)
		EEXIT("starting key-cycle timer\n");

	modifier_state = mods;
	cycle_complete = complete;
}

void keys_cycle_complete(void){
	if(cycle_complete != 0x0)
		cycle_complete();

	modifier_state = 0;
	cycle_complete = 0x0;

	if(timer_set(modifier_reset_timer, 0) != 0)
		EEXIT("stopping key-cycle timer\n");
}

bool keys_cycle_active(void){
	return (modifier_state != 0);
}


/* local functions */
static int modifier_reset_hdlr(void){
	timer_ack(modifier_reset_timer);
	modifier_state &= input_get_mod_state();

	if(modifier_state == 0)
		keys_cycle_complete();

	return 0;
}
