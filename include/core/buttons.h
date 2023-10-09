#ifndef BUTTONS_H
#define BUTTONS_H


#include <xlib/input.h>
#include <utils/utils.h>


/* macros */
#define BUTTON_N(_id, _button, _mods, _click, _action, ...) \
	static button_map_t const button_##_id \
		linker_array("buttons") = { \
			.click = _click, \
			.mask = _mods, \
			.button = _button, \
			.func = _action, \
			.arg = { __VA_ARGS__ }, \
		}

#define BUTTON(button, mods, click, action, ...)	UNIQUE(BUTTON_N, __COUNTER__, button, mods, click, action, __VA_ARGS__)


/* global functions */
void buttons_handle(click_t click, unsigned int button, unsigned int mods);


/* external variables */
extern button_map_t __start_buttons[],
					__stop_buttons[];


#endif // BUTTONS_H
