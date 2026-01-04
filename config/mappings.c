#include <config/config.h>
#include <X11/X.h>
#include <X11/keysymdef.h>
#include <X11/XF86keysym.h>
#include <core/buttons.h>
#include <core/keys.h>
#include <commands.h>


/* macros */
#define ALT		Mod1Mask
#define WIN		Mod4Mask
#define MODKEY	WIN

#define MOVE_INC	CONFIG_WIN_MOVE_DELTA
#define SIZE_INC	CONFIG_WIN_RESIZE_DELTA

#define TAGKEY(keysym, tag) \
	KEY(keysym,	ControlMask,				cmd_tags_view,			.ui = (1 << tag)); \
	KEY(keysym,	ControlMask | ShiftMask,	cmd_tags_toggle,		.ui = (1 << tag)); \
	KEY(keysym,	MODKEY,						cmd_tags_client_set,	.ui = (1 << tag)); \
	KEY(keysym,	MODKEY | ShiftMask,			cmd_tags_client_toggle,	.ui = (1 << tag))

#define SPOTIFY(op)	\
	SPAWN(\
		"dbus-send", \
		"--print-reply", \
		"--dest=org.mpris.MediaPlayer2.spotify", \
		"/org/mpris/MediaPlayer2", \
		"org.mpris.MediaPlayer2.Player." op \
	)


/* keys */
// tag control
TAGKEY(XK_F1, 0);
TAGKEY(XK_F2, 1);
TAGKEY(XK_F3, 2);
TAGKEY(XK_F4, 3);

// dwm control
KEY(XK_F4,		ALT,				cmd_client_kill,		0);
KEY(XK_Tab,		ALT,				cmd_client_cycle,		.i = +1);
KEY(XK_Tab,		ALT | ShiftMask,	cmd_client_cycle,		.i = -1);
KEY(XK_Delete,	ALT | ControlMask,	cmd_lifecycle,			.i = -1);
KEY(XK_s,		MODKEY,				cmd_statusbar_toggle,	0);
KEY(XK_F2,		ALT,				cmd_dmenu_run,			0);

// window movement
KEY(XK_Up,		MODKEY,				cmd_client_move,	.v = (int[]){ 0, -MOVE_INC });
KEY(XK_Down,	MODKEY,				cmd_client_move,	.v = (int[]){ 0, MOVE_INC });
KEY(XK_Left,	MODKEY,				cmd_client_move,	.v = (int[]){ -MOVE_INC, 0 });
KEY(XK_Right,	MODKEY,				cmd_client_move,	.v = (int[]){ MOVE_INC, 0 });
KEY(XK_Up,		MODKEY | ShiftMask,	cmd_client_move,	.v = (int[]){ 0, -INT_MAX });
KEY(XK_Down,	MODKEY | ShiftMask,	cmd_client_move,	.v = (int[]){ 0, INT_MAX });
KEY(XK_Left,	MODKEY | ShiftMask,	cmd_client_move,	.v = (int[]){ -INT_MAX, 0 });
KEY(XK_Right,	MODKEY | ShiftMask,	cmd_client_move,	.v = (int[]){ INT_MAX, 0 });

// window size
KEY(XK_Next,	MODKEY,						cmd_client_resize,		.v = (int []){ 0, SIZE_INC });
KEY(XK_Prior,	MODKEY,						cmd_client_resize,		.v = (int []){ 0, -SIZE_INC });
KEY(XK_End,		MODKEY,						cmd_client_resize,		.v = (int []){ SIZE_INC, 0 });
KEY(XK_Home,	MODKEY,						cmd_client_resize,		.v = (int []){ -SIZE_INC, 0 });
KEY(XK_Down,	MODKEY | ALT | ShiftMask,	cmd_client_max,			.v = (int []){ 0, 1 });
KEY(XK_Up,		MODKEY | ALT | ShiftMask,	cmd_client_max,			.v = (int []){ 0, 1 });
KEY(XK_Right,	MODKEY | ALT | ShiftMask,	cmd_client_max,			.v = (int []){ 1, 0 });
KEY(XK_Left,	MODKEY | ALT | ShiftMask,	cmd_client_max,			.v = (int []){ 1, 0 });
KEY(XK_Insert,	MODKEY,						cmd_client_max,			.v = (int []){ 1, 1});
KEY(XK_Insert,	MODKEY | ShiftMask,			cmd_client_fullscreen,	0);

// winfade
KEY(XK_1,		MODKEY,				cmd_winfade_fade,	.ui = (1 << 0));
KEY(XK_2,		MODKEY,				cmd_winfade_fade,	.ui = (1 << 1));
KEY(XK_3,		MODKEY,				cmd_winfade_fade,	.ui = (1 << 2));
KEY(XK_1,		MODKEY | ShiftMask,	cmd_winfade_add,	.ui = (1 << 0));
KEY(XK_2,		MODKEY | ShiftMask,	cmd_winfade_add,	.ui = (1 << 1));
KEY(XK_3,		MODKEY | ShiftMask,	cmd_winfade_add,	.ui = (1 << 2));

// keylock window
KEY(XK_l,		MODKEY,				cmd_keylock_toggle,	0);
KEY(XK_l,		MODKEY | ShiftMask,	cmd_keylock_set,	0);

// zaphod
KEY(XK_z,		MODKEY,	cmd_zaphod_toggle,	0);

// audio control
KEY(XF86XK_AudioMute,			0,	cmd_spawn,	.v = SPAWN("st-audio", "mute", "toggle"));
KEY(XF86XK_AudioRaiseVolume,	0,	cmd_spawn,	.v = SPAWN("st-audio", "volume", "raise"));
KEY(XF86XK_AudioLowerVolume,	0,	cmd_spawn,	.v = SPAWN("st-audio", "volume", "lower"));

// display and keyboard backlight control
KEY(XF86XK_MonBrightnessDown,	0,			cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "display-backlight", "-50"));
KEY(XF86XK_MonBrightnessUp,		0,			cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "display-backlight", "+50"));
KEY(XF86XK_MonBrightnessDown,	ShiftMask,	cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "keyboard-backlight", "-1"));
KEY(XF86XK_MonBrightnessUp,		ShiftMask,	cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "keyboard-backlight", "+1"));

// fan control
KEY(XF86XK_AudioMute,			ShiftMask,	cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "fan", "t"));
KEY(XF86XK_AudioRaiseVolume,	ShiftMask,	cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "fan", "+"));
KEY(XF86XK_AudioLowerVolume,	ShiftMask,	cmd_spawn,	.v = SPAWN("statdctrl", "ctrl", "fan", "-"));

// media control
KEY(XF86XK_AudioPlay,			0,			cmd_spawn,	.v = SPOTIFY("PlayPause"));
KEY(XF86XK_AudioPrev,			0,			cmd_spawn,	.v = SPOTIFY("Previous"));
KEY(XF86XK_AudioNext,			0,			cmd_spawn,	.v = SPOTIFY("Next"));


/* buttons */
BUTTON(Button1, ALT,				BLOC_CLIENT,	0x0 /* focus client */,		0);
BUTTON(Button1,	MODKEY,				BLOC_CLIENT,	cmd_client_move_mouse,		0);
BUTTON(Button1,	MODKEY | ShiftMask,	BLOC_CLIENT,	cmd_client_resize_mouse,	0);
BUTTON(Button1,	0,					BLOC_LAYOUT,	cmd_layout_select,			0);
BUTTON(Button1,	0,					BLOC_TAGBAR,	cmd_tags_menu,				0);
BUTTON(Button1,	0,					BLOC_LAUNCHER,	cmd_launcher_menu,			0);
