/* See LICENSE file for copyright and license details. */

#include <action.h>

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};
unsigned int ncolors = LENGTH(colors);

/* tagging */
const char *tags[] = { "💻", "💻", "🗇", "📟" };
unsigned int ntags = LENGTH(tags);

static const rule_t rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const layout_t layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

/* key definitions */
#define ALT		Mod1Mask
#define WIN		Mod4Mask
#define MODKEY	WIN

	// move to tag
	// move window to tag
	// toggle to view windows for tag
	// toggle add window to tag
#define TAGKEYS(KEY,TAG) \
	{ ControlMask,				KEY,	action_view,		{.ui = 1 << TAG} }, \
	{ MODKEY,					KEY,	action_tag,			{.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,		KEY,	action_toggleview,	{.ui = 1 << TAG} }, \
	{ ControlMask|ShiftMask,	KEY,	action_toggletag,	{.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", CONFIG_FONT, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };

static const key_map_t keys[] = {
	// tag control
	TAGKEYS(					XK_F1,	0)
	TAGKEYS(					XK_F2,	1)
	TAGKEYS(					XK_F3,	2)
	TAGKEYS(					XK_F4,	3)

	// window movement
	{ MODKEY,					XK_Up,		action_moveclient,	{ .v = (int[]){ 0, -30 } } },
	{ MODKEY,					XK_Down,	action_moveclient,	{ .v = (int[]){ 0, 30 } } },
	{ MODKEY,					XK_Left,	action_moveclient,	{ .v = (int[]){ -40, 0 } } },
	{ MODKEY,					XK_Right,	action_moveclient,	{ .v = (int[]){ 40, 0 } } },
	{ MODKEY | ShiftMask,		XK_Up,		action_moveclient,	{ .v = (int[]){ 0, -INT_MAX } } },
	{ MODKEY | ShiftMask,		XK_Down,	action_moveclient,	{ .v = (int[]){ 0, INT_MAX } } },
	{ MODKEY | ShiftMask,		XK_Left,	action_moveclient,	{ .v = (int[]){ -INT_MAX, 0 } } },
	{ MODKEY | ShiftMask,		XK_Right,	action_moveclient,	{ .v = (int[]){ INT_MAX, 0 } } },

	// window size
	{ MODKEY,					XK_Next,	action_reszclient,	{ .v = (int []){ 0, 25 } } },
	{ MODKEY,					XK_Prior,	action_reszclient,	{ .v = (int []){ 0, -25 } } },
	{ MODKEY,					XK_End,		action_reszclient,	{ .v = (int []){ 25, 0 } } },
	{ MODKEY,					XK_Home,	action_reszclient,	{ .v = (int []){ -25, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Down,	action_reszclient,	{ .v = (int []){ 0, INT_MAX } } },
	{ MODKEY | ALT | ShiftMask,	XK_Up,		action_reszclient,	{ .v = (int []){ 0, INT_MAX } } },
	{ MODKEY | ALT | ShiftMask,	XK_Right,	action_reszclient,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Left,	action_reszclient,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Left,	action_reszclient,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY,					XK_Insert,	action_reszclient,	{ .v = (int []){ INT_MAX, INT_MAX } } },

	// window open/close/focus
	{ ALT,						XK_F4,		action_killclient,	{ 0 } },
	{ ALT,						XK_Tab,		action_focusstack,	{ .i = +1 } },
	{ ALT | ShiftMask,			XK_Tab,		action_focusstack,	{ .i = -1 } },

	// dwm start/stop
	// TODO call dmenu as menu asking for exit | restart
	{ MODKEY|ShiftMask,			XK_q,		action_quit,		{ 0 } },
	{ ALT | ControlMask,		XK_Delete,	action_restart,		{ 0 } },

	// launch programs
	{ MODKEY,					XK_s,		action_togglebar,	{ 0 } },
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
//	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
//	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
//	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
//	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
//	{ MODKEY,                       XK_Return, zoom,           {0} },
//	{ MODKEY,                       XK_Tab,    view,           {0} },
//	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
//	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
//	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
//	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
//	{ MODKEY,                       XK_space,  setlayout,      {0} },
//	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
//	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
//	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
//	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
//	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
//	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
//	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const button_map_t buttons[] = {
	// window movement/size
	{ ClkClientWin,	MODKEY,				Button1,	action_movemouse,	{0} },
	{ ClkClientWin,	MODKEY | ShiftMask,	Button1,	action_resizemouse,	{0} },

	// status bar actions
	{ ClkLtSymbol,	0,					Button1,	action_setlayout,	{0} },
	{ ClkLtSymbol,	0,					Button3,	action_setlayout,	{.v = &layouts[2]} },
	{ ClkWinTitle,	0,					Button2,	action_zoom,		{0} },
	{ ClkTagBar,	0,					Button1,	action_view,		{0} },
	{ ClkTagBar,	0,					Button3,	action_toggleview,	{0} },
	{ ClkTagBar,	MODKEY,				Button1,	action_tag,			{0} },
	{ ClkTagBar,	MODKEY,				Button3,	action_toggletag,	{0} },
};
