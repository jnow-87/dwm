#ifndef DWM_H
#define DWM_H


#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <drw.h>
#include <monitor.h>
#include <statusbar.h>
#include <layout.h>


/* types */
typedef enum{
	CurNormal,
	CurResize,
	CurMove,
	CurLast
} cursor_type_t;

typedef enum{
	NetSupported,
	NetWMName,
	NetWMCheck,
	NetActiveWindow,
	NetClientList,
	NetLast
} net_atom_t;

typedef enum{
	WMProtocols,
	WMDelete,
	WMState,
	WMTakeFocus,
	WMLast
} default_atoms_t;

typedef struct{
	monitor_t *mons;
	int screen_width,
		screen_height;
	int lrpad;	/* sum of left and right padding for text */
	int running;
	unsigned int numlock_mask;
	drw_t *drw;

	Display *dpy;
	int screen;
	Atom netatom[NetLast];
	Atom wmatom[WMLast];
	Window root;

	int (*xlib_xerror_hdlr)(Display *, XErrorEvent *); // default error handler used by xlib

	color_t **scheme;
	cursor_t *cursor[CurLast];
	statusbar_t statusbar;
	layout_t const *layout;
} dwm_t;


/* prototypes */
void die(char const *fmt, ...);
int getrootptr(int *x, int *y);
void grabkeys(void);
int monitor_discover(void);
void updatenumlockmask(void);


/* external variables */
extern dwm_t dwm;


#endif // DWM_H
