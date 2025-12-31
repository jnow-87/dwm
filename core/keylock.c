#include <stdbool.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <utils/utils.h>
#include <commands.h>


/* global functions */
bool keylock_active(void){
	return (dwm.keylock != 0x0 && dwm.keylock == dwm.focused);
}

bool keylock_key_match(keysym_t sym, unsigned int mods){
	keymap_t *key;


	config_for_each(keys, key){
		if(keys_registered(key, sym, mods) && (key->action == cmd_keylock_set || key->action == cmd_keylock_toggle))
			return true;
	}

	return false;
}
