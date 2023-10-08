#ifndef BUTTONS_H
#define BUTTONS_H


/* types */
typedef enum{
	CLK_UNKNOWN = -1,
	CLK_ROOT = 0,
	CLK_CLIENT,
	CLK_TAGBAR,
	CLK_LAYOUT,
	CLK_STATUS,
} click_t;


/* global functions */
void buttons_handle(click_t click, unsigned int button, unsigned int mods);


#endif // BUTTONS_H
