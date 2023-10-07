#ifndef DWM_H
#define DWM_H


#include <X11/X.h>
#include <xlib/gfx.h>
#include <xlib/atoms.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/layout.h>


/* types */
typedef enum{
	CurNormal,
	CurResize,
	CurMove,
	CurLast
} cursor_type_t;

typedef struct{
	monitor_t *mons;
	int screen_width,
		screen_height;
	int lrpad;	/* sum of left and right padding for text */
	unsigned int numlock_mask;
	gfx_t *gfx;
	Display *dpy;
	int screen;
	Atom netatom[NetLast];
	Atom wmatom[WMLast];
	Window root;

	int (*xlib_xerror_hdlr)(Display *, XErrorEvent *); // default error handler used by xlib

	int running;
	int event_fd;

	client_t *stack,
			 *focused;

	color_t **scheme;
	cursor_t *cursor[CurLast];
	statusbar_t statusbar;
	layout_t const *layout;
	unsigned int tag_mask;
} dwm_t;

typedef int (*event_hdlr_t)(void);


/* prototypes */
int dwm_setup(void);
void dwm_cleanup(void);
void dwm_run(void);
void dwm_die(char const *fmt, ...);

int dwm_hdlr_add(int fd, event_hdlr_t hdlr);


/* external variables */
extern dwm_t dwm;


#endif // DWM_H
