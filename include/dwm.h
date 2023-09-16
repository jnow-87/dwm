#ifndef DWM_H
#define DWM_H


#include <X11/X.h>
#include <X11/Xlib.h>
#include <drw.h>


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
	NetWMState,
	NetWMCheck,
	NetWMFullscreen,
	NetActiveWindow,
	NetWMWindowType,
	NetWMWindowTypeDialog,
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


/* prototypes */
void die(const char *fmt, ...);
int getrootptr(int *x, int *y);
int gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabkeys(void);
void manage(Window w, XWindowAttributes *wa);
void updateclientlist(void);
int updategeom(void);
void updatenumlockmask(void);


/* external variables */
extern const int lockfullscreen;
extern int running;
extern const char *tags[];
extern unsigned int ntags;	// TODO replace with LENGTH(tags)
extern const char *dmenucmd[];
extern char dmenumon[];
extern int bar_height;
extern int (*xlib_xerror_hdlr)(Display *, XErrorEvent *);	// default error handler used by xlib
extern int screen;
extern int sw, sh;           /* X display screen geometry width, height */
extern int lrpad;            /* sum of left and right padding for text */
extern unsigned int numlockmask;
extern Atom netatom[NetLast];
extern Atom wmatom[];
extern Display *dpy;
extern Window root;
extern color_t **scheme;
extern cursor_t *cursor[];
extern drw_t *drw;


#endif // DWM_H
