/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* tagging */
static const char *tags[] = { "💻", "💻", "🗇", "📟" };

static const Rule rules[] = {
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
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */

static const Layout layouts[] = {
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
	{ ControlMask,				KEY,	view,		{.ui = 1 << TAG} }, \
	{ MODKEY,					KEY,	tag,		{.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,		KEY,	toggleview,	{.ui = 1 << TAG} }, \
	{ ControlMask|ShiftMask,	KEY,	toggletag,	{.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", CONFIG_FONT, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };

static const Key keys[] = {
	// tag control
	TAGKEYS(					XK_F1,	0)
	TAGKEYS(					XK_F2,	1)
	TAGKEYS(					XK_F3,	2)
	TAGKEYS(					XK_F4,	3)

	// window movement
	{ MODKEY,					XK_Up,		moveclient,	{ .v = (int[]){ 0, -30 } } },
	{ MODKEY,					XK_Down,	moveclient,	{ .v = (int[]){ 0, 30 } } },
	{ MODKEY,					XK_Left,	moveclient,	{ .v = (int[]){ -40, 0 } } },
	{ MODKEY,					XK_Right,	moveclient,	{ .v = (int[]){ 40, 0 } } },
	{ MODKEY | ShiftMask,		XK_Up,		moveclient,	{ .v = (int[]){ 0, -INT_MAX } } },
	{ MODKEY | ShiftMask,		XK_Down,	moveclient,	{ .v = (int[]){ 0, INT_MAX } } },
	{ MODKEY | ShiftMask,		XK_Left,	moveclient,	{ .v = (int[]){ -INT_MAX, 0 } } },
	{ MODKEY | ShiftMask,		XK_Right,	moveclient,	{ .v = (int[]){ INT_MAX, 0 } } },

	// window size
	{ MODKEY,					XK_Next,	reszclient,	{ .v = (int []){ 0, 25 } } },
	{ MODKEY,					XK_Prior,	reszclient,	{ .v = (int []){ 0, -25 } } },
	{ MODKEY,					XK_End,		reszclient,	{ .v = (int []){ 25, 0 } } },
	{ MODKEY,					XK_Home,	reszclient,	{ .v = (int []){ -25, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Down,	reszclient,	{ .v = (int []){ 0, INT_MAX } } },
	{ MODKEY | ALT | ShiftMask,	XK_Up,		reszclient,	{ .v = (int []){ 0, INT_MAX } } },
	{ MODKEY | ALT | ShiftMask,	XK_Right,	reszclient,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Left,	reszclient,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY | ALT | ShiftMask,	XK_Left,	reszclient,	{ .v = (int []){ INT_MAX, 0 } } },
	{ MODKEY,					XK_Insert,	reszclient,	{ .v = (int []){ INT_MAX, INT_MAX } } },

	// window open/close/focus
	{ ALT,					XK_F4,		killclient,	{ 0 } },
	{ ALT,					XK_Tab,		focusstack,	{ .i = +1 } },
	{ ALT | ShiftMask,		XK_Tab,		focusstack,	{ .i = -1 } },

	// dwm start/stop
	{ MODKEY|ShiftMask,		XK_q,		quit,		{ 0 } },
	{ ALT | ControlMask,	XK_Delete,	restart,	{ 0 } },

	// launch programs
	{ MODKEY,				XK_s,		togglebar,		{ 0 } },
	{ ALT,					XK_F2,		spawn,			{ .v = dmenucmd } },

	// winfade
	{ MODKEY,				XK_1,	spawn,			{ .v = (char const *[]){ "winfade", "--group", "1", "fade" } } },
	{ MODKEY,				XK_2,	spawn,			{ .v = (char const *[]){ "winfade", "--group", "2", "fade" } } },
	{ MODKEY,				XK_3,	spawn,			{ .v = (char const *[]){ "winfade", "--group", "3", "fade" } } },
	{ MODKEY | ShiftMask,	XK_1,	spawn,			{ .v = (char const *[]){ "winfade", "--group", "1", "select" } } },
	{ MODKEY | ShiftMask,	XK_2,	spawn,			{ .v = (char const *[]){ "winfade", "--group", "2", "select" } } },
	{ MODKEY | ShiftMask,	XK_3,	spawn,			{ .v = (char const *[]){ "winfade", "--group", "3", "select" } } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	// window movement/size
	{ ClkClientWin,	MODKEY,				Button1,	movemouse,		{0} },
	{ ClkClientWin,	MODKEY | ShiftMask,	Button1,	resizemouse,	{0} },

	// status bar actions
	{ ClkLtSymbol,	0,					Button1,	setlayout,		{0} },
	{ ClkLtSymbol,	0,					Button3,	setlayout,		{.v = &layouts[2]} },
	{ ClkWinTitle,	0,					Button2,	zoom,			{0} },
	{ ClkTagBar,	0,					Button1,	view,			{0} },
	{ ClkTagBar,	0,					Button3,	toggleview,		{0} },
	{ ClkTagBar,	MODKEY,				Button1,	tag,			{0} },
	{ ClkTagBar,	MODKEY,				Button3,	toggletag,		{0} },
};
