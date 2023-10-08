#ifndef DWM_H
#define DWM_H


#include <X11/X.h>
#include <xlib/gfx.h>
#include <xlib/atoms.h>
#include <xlib/input.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/layout.h>


/* macros */
#define CLEANMODS(mods) (mods & ~(dwm.numlock_mask | LOCKS_MASK) & (MODS_MASK))


/* types */
typedef enum{
	DWM_ERROR = -1,
	DWM_SHUTDOWN,
	DWM_RUN,
	DWM_RESTART,
} dwm_state_t;

typedef struct{
	// x-server connection
	Display *dpy;
	Window root,
		   wmcheck;

	int screen;
	int screen_width,
		screen_height;

	unsigned int numlock_mask;

	Atom netatom[NetLast];
	Atom wmatom[WMLast];

	// graphics
	gfx_t *gfx;
	cursor_t cursor[CurLast];
	color_t **scheme;
	int lrpad;	/* sum of left and right padding for text */

	// dwm lifecycle
	dwm_state_t state;
	int event_fd;

	// dwm components
	monitor_t *mons;
	statusbar_t statusbar;
	layout_t const *layout;

	// client handling
	client_t *stack,
			 *focused;

	unsigned int tag_mask;
} dwm_t;

typedef int (*event_hdlr_t)(void);


/* prototypes */
int dwm_setup(void);
void dwm_cleanup(void);
void dwm_run(void);

int dwm_hdlr_add(int fd, event_hdlr_t hdlr);


/* external variables */
extern dwm_t dwm;


#endif // DWM_H
