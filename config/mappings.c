#include <config/config.h>
#include <stddef.h>
#include <limits.h>
#include <X11/X.h>
#include <X11/keysymdef.h>
#include <core/buttons.h>
#include <core/keys.h>
#include <utils/utils.h>
#include <actions.h>


/* macros */
/* key definitions */
#define ALT		Mod1Mask
#define WIN		Mod4Mask
#define MODKEY	WIN

	// move to tag
	// move window to tag
	// toggle to view windows for tag
	// toggle add window to tag
#define TAGKEYS(KEY, TAG) \
	{ ControlMask,				KEY,	action_tags_view,			{.ui = 1 << TAG} }, \
	{ ControlMask | ShiftMask,	KEY,	action_tags_toggle,			{.ui = 1 << TAG} }, \
	{ MODKEY,					KEY,	action_tags_client_set,		{.ui = 1 << TAG} }, \
	{ MODKEY | ShiftMask,		KEY,	action_tags_client_toggle,	{.ui = 1 << TAG} },


/* global variables */
char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
char const *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", CONFIG_FONT, "-nb", CONFIG_COL_INACT_BG, "-nf", CONFIG_COL_INACT_FG, "-sb", CONFIG_COL_ACT_BG, "-sf", CONFIG_COL_ACT_FG, 0x0 };

key_map_t const keys[] = {
	// tag control
	TAGKEYS(					XK_F1,	0)
	TAGKEYS(					XK_F2,	1)
	TAGKEYS(					XK_F3,	2)
	TAGKEYS(					XK_F4,	3)

	// window movement
	{ MODKEY,					XK_Up,		action_client_move,	{ .v = (int[]){ 0, -30 } } },
	{ MODKEY,					XK_Down,	action_client_move,	{ .v = (int[]){ 0, 30 } } },
	{ MODKEY,					XK_Left,	action_client_move,	{ .v = (int[]){ -40, 0 } } },
	{ MODKEY,					XK_Right,	action_client_move,	{ .v = (int[]){ 40, 0 } } },
	{ MODKEY | ShiftMask,		XK_Up,		action_client_move,	{ .v = (int[]){ 0, -INT_MAX } } },
	{ MODKEY | ShiftMask,		XK_Down,	action_client_move,	{ .v = (int[]){ 0, INT_MAX } } },
	{ MODKEY | ShiftMask,		XK_Left,	action_client_move,	{ .v = (int[]){ -INT_MAX, 0 } } },
	{ MODKEY | ShiftMask,		XK_Right,	action_client_move,	{ .v = (int[]){ INT_MAX, 0 } } },

	// window size
	{ MODKEY,					XK_Next,	action_client_resize,	{ .v = (int []){ 0, 25 } } },
	{ MODKEY,					XK_Prior,	action_client_resize,	{ .v = (int []){ 0, -25 } } },
	{ MODKEY,					XK_End,		action_client_resize,	{ .v = (int []){ 25, 0 } } },
	{ MODKEY,					XK_Home,	action_client_resize,	{ .v = (int []){ -25, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Down,	action_client_resize,	{ .v = (int []){ 0, INT_MAX } } },
	{ MODKEY | ALT | ShiftMask,	XK_Up,		action_client_resize,	{ .v = (int []){ 0, INT_MAX } } },
	{ MODKEY | ALT | ShiftMask,	XK_Right,	action_client_resize,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Left,	action_client_resize,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Left,	action_client_resize,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY,					XK_Insert,	action_client_resize,	{ .v = (int []){ INT_MAX, INT_MAX } } },

	// window open/close/client_focus
	{ ALT,						XK_F4,		action_client_kill,	{ 0 } },
	{ ALT,						XK_Tab,		action_client_cycle,	{ .i = +1 } },
	{ ALT | ShiftMask,			XK_Tab,		action_client_cycle,	{ .i = -1 } },

	// dwm start/stop
	// TODO call dmenu as menu asking for exit | restart
	{ MODKEY|ShiftMask,			XK_q,		action_quit,		{ 0 } },
	{ ALT | ControlMask,		XK_Delete,	action_restart,		{ 0 } },

	// launch programs
	{ MODKEY,					XK_s,		action_statusbar_toggle,	{ 0 } },
	{ ALT,						XK_F2,		action_spawn,		{ .v = dmenucmd } },

	// TODO pstree comparing starting xterm via spawn and via dmenu
//	{ MODKEY,				XK_x,		spawn,			{ .v = termcmd } },

	// winfade
	// TODO dwm doesn't set _NET_CURRENT_DESKTOP
	{ MODKEY,					XK_1,		action_spawn,		{ .v = (char const *[]){ "winfade", "--group", "1", "fade" } } },
	{ MODKEY,					XK_2,		action_spawn,		{ .v = (char const *[]){ "winfade", "--group", "2", "fade" } } },
	{ MODKEY,					XK_3,		action_spawn,		{ .v = (char const *[]){ "winfade", "--group", "3", "fade" } } },
	{ MODKEY | ShiftMask,		XK_1,		action_spawn,		{ .v = (char const *[]){ "winfade", "--group", "1", "select" } } },
	{ MODKEY | ShiftMask,		XK_2,		action_spawn,		{ .v = (char const *[]){ "winfade", "--group", "2", "select" } } },
	{ MODKEY | ShiftMask,		XK_3,		action_spawn,		{ .v = (char const *[]){ "winfade", "--group", "3", "select" } } },
//	{ MODKEY,                       XK_Return, zoom,           {0} },
//	{ MODKEY,                       XK_Tab,    view,           {0} },
//	{ MODKEY|ShiftMask,             XK_c,      win_kill,     {0} },
//	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
//	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
//	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
//	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
//	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
//	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
//	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
};

size_t nkeys = LENGTH(keys);

/* click can be CLK_TAGBAR, CLK_LAYOUT, CLK_STATUS, ClkWinTitle, CLK_CLIENT, or CLK_ROOT */
button_map_t const buttons[] = {
	// window movement/size
	{ CLK_CLIENT,	MODKEY,				Button1,	action_client_move_mouse,	{0} },
	{ CLK_CLIENT,	MODKEY | ShiftMask,	Button1,	action_client_resize_mouse,	{0} },

	// status bar actions
	{ CLK_LAYOUT,	0,					Button1,	action_layout_select,	{0} },
};

size_t nbuttons = LENGTH(buttons);
