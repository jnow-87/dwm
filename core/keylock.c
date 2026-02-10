#include <stdbool.h>
#include <core/dwm.h>
#include <core/keys.h>
#include <utils/vector.h>
#include <commands.h>
#include <rc.h>


/* global functions */
void keylock_set(client_t *c){
	dwm.keylock = c;

	statusbar_update();
}

bool keylock_active(void){
	return (dwm.keylock != 0x0 && dwm.keylock == dwm.focused);
}

bool keylock_key_match(keysym_t sym, unsigned int mods){
	keymap_t *key;


	vector_for_each(&rc.keys, key){
		if(keys_registered(key, sym, mods) && (key->action == cmd_keylock_set || key->action == cmd_keylock_toggle))
			return true;
	}

	return false;
}
