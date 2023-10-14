#include <X11/X.h>
#include <X11/keysymdef.h>
#include <core/buttons.h>
#include <core/keys.h>
#include <actions.h>


/* macros */
#define ALT		Mod1Mask
#define WIN		Mod4Mask
#define MODKEY	WIN

#define MOVE_INC	100
#define SIZE_INC	50

#define TAGKEY(keysym, tag) \
	KEY(keysym,	ControlMask,				action_tags_view,			.ui = (1 << tag)); \
	KEY(keysym,	ControlMask | ShiftMask,	action_tags_toggle,			.ui = (1 << tag)); \
	KEY(keysym,	MODKEY,						action_tags_client_set,		.ui = (1 << tag)); \
	KEY(keysym,	MODKEY | ShiftMask,			action_tags_client_toggle,	.ui = (1 << tag))


/* keys */
// tag control
TAGKEY(XK_F1, 0);
TAGKEY(XK_F2, 1);
TAGKEY(XK_F3, 2);
TAGKEY(XK_F4, 3);

// dwm control
KEY(XK_F4,		ALT,				action_client_kill,			0);
KEY(XK_Tab,		ALT,				action_client_cycle,		.i = +1);
KEY(XK_Tab,		ALT | ShiftMask,	action_client_cycle,		.i = -1);
KEY(XK_Delete,	ALT | ControlMask,	action_lifecycle,			.i = -1);
KEY(XK_s,		MODKEY,				action_statusbar_toggle,	0);
KEY(XK_F2,		ALT,				action_dmenu_run,			0);

// window movement
KEY(XK_Up,		MODKEY,				action_client_move,	.v = (int[]){ 0, -MOVE_INC });
KEY(XK_Down,	MODKEY,				action_client_move,	.v = (int[]){ 0, MOVE_INC });
KEY(XK_Left,	MODKEY,				action_client_move,	.v = (int[]){ -MOVE_INC, 0 });
KEY(XK_Right,	MODKEY,				action_client_move,	.v = (int[]){ MOVE_INC, 0 });
KEY(XK_Up,		MODKEY | ShiftMask,	action_client_move,	.v = (int[]){ 0, -INT_MAX });
KEY(XK_Down,	MODKEY | ShiftMask,	action_client_move,	.v = (int[]){ 0, INT_MAX });
KEY(XK_Left,	MODKEY | ShiftMask,	action_client_move,	.v = (int[]){ -INT_MAX, 0 });
KEY(XK_Right,	MODKEY | ShiftMask,	action_client_move,	.v = (int[]){ INT_MAX, 0 });

// window size
KEY(XK_Next,	MODKEY,						action_client_resize,	.v = (int []){ 0, SIZE_INC });
KEY(XK_Prior,	MODKEY,						action_client_resize,	.v = (int []){ 0, -SIZE_INC });
KEY(XK_End,		MODKEY,						action_client_resize,	.v = (int []){ SIZE_INC, 0 });
KEY(XK_Home,	MODKEY,						action_client_resize,	.v = (int []){ -SIZE_INC, 0 });
KEY(XK_Down,	MODKEY | ALT | ShiftMask,	action_client_resize,	.v = (int []){ 0, INT_MAX });
KEY(XK_Up,		MODKEY | ALT | ShiftMask,	action_client_resize,	.v = (int []){ 0, INT_MAX });
KEY(XK_Right,	MODKEY | ALT | ShiftMask,	action_client_resize,	.v = (int []){ INT_MAX, 0 });
KEY(XK_Left,	MODKEY | ALT | ShiftMask,	action_client_resize,	.v = (int []){ INT_MAX, 0 });
KEY(XK_Left,	MODKEY | ALT | ShiftMask,	action_client_resize,	.v = (int []){ INT_MAX, 0 });
KEY(XK_Insert,	MODKEY,						action_client_resize,	.v = (int []){ INT_MAX, INT_MAX });

// winfade
// TODO dwm doesn't set _NET_CURRENT_DESKTOP
KEY(XK_1,		MODKEY,				action_spawn,	.v = (char const *[]){ "winfade", "--group", "1", "fade" });
KEY(XK_2,		MODKEY,				action_spawn,	.v = (char const *[]){ "winfade", "--group", "2", "fade" });
KEY(XK_3,		MODKEY,				action_spawn,	.v = (char const *[]){ "winfade", "--group", "3", "fade" });
KEY(XK_1,		MODKEY | ShiftMask,	action_spawn,	.v = (char const *[]){ "winfade", "--group", "1", "select" });
KEY(XK_2,		MODKEY | ShiftMask,	action_spawn,	.v = (char const *[]){ "winfade", "--group", "2", "select" });
KEY(XK_3,		MODKEY | ShiftMask,	action_spawn,	.v = (char const *[]){ "winfade", "--group", "3", "select" });


/* buttons */
BUTTON(Button1, 0,					BLOC_CLIENT,	0x0 /* focus client */,		0);
BUTTON(Button1,	MODKEY,				BLOC_CLIENT,	action_client_move_mouse,	0);
BUTTON(Button1,	MODKEY | ShiftMask,	BLOC_CLIENT,	action_client_resize_mouse,	0);
BUTTON(Button1,	0,					BLOC_LAYOUT,	action_layout_select,		0);
