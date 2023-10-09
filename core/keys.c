#include <config.h>
#include <stdint.h>
#include <unistd.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <xlib/input.h>
#include <utils/timer.h>


/* local/static prototypes */
static int modifier_reset_hdlr(void);


/* static variables */
static unsigned int modifier_state = 0;
static int modifier_reset_timer = -1;
static cycle_callback_t cycle_complete = 0x0;


/* global functions */
int keys_init(void){
	modifier_reset_timer = timer_init();

	if(modifier_reset_timer == -1)
		return -1;

	return dwm_hdlr_add(modifier_reset_timer, modifier_reset_hdlr);
}

void keys_cleanup(void){
	if(modifier_reset_timer != -1)
		close(modifier_reset_timer);
}

void keys_handle(keysym_t sym, unsigned int mods){
	for(size_t i=0; i<nkeys; i++){
		if(sym == keys[i].keysym && CLEANMODS(keys[i].mod) == CLEANMODS(mods) && keys[i].func)
			keys[i].func(&(keys[i].arg));
	}
}

void keys_cycle_start(cycle_callback_t complete){
	unsigned int mods;


	mods = input_get_mod_state();

	// TODO should complete() be called if the cycle is completed immediately
	if(mods == 0)
		return;

	keys_cycle_complete();

	// the xlib KeyRelease event does not reliably report the release of modifier keys,
	// hence use a timer to reset the modifier state manually
	if(timer_set(modifier_reset_timer, 100) != 0)
		dwm_die("unable to start key-clientstack_cycle timer\n");

	modifier_state = mods;
	cycle_complete = complete;
}

void keys_cycle_complete(void){
	if(cycle_complete != 0x0)
		cycle_complete();

	modifier_state = 0;
	cycle_complete = 0x0;

	if(timer_set(modifier_reset_timer, 0) != 0)
		dwm_die("unable to stop key-clientstack_cycle timer\n");
}

bool keys_cycle_active(void){
	return (modifier_state != 0);
}


/* local functions */
static int modifier_reset_hdlr(void){
	uint64_t data;


	read(modifier_reset_timer, &data, sizeof(data));
	modifier_state &= input_get_mod_state();

	if(modifier_state == 0)
		keys_cycle_complete();

	return 0;
}
