/* See LICENSE file for copyright and license details. */

#ifndef DRW_H
#define DRW_H


#include <X11/X.h>
#include <X11/Xft/Xft.h>


/* macros */
#define TEXTW(X)	(drw_fontset_getwidth(dwm.drw, (X)) + dwm.lrpad)


/* types */
typedef enum{
	ColFg,
	ColBg,
	ColBorder
} color_scheme_index_t;

typedef XftColor color_t;

typedef struct{
	Cursor cursor;
} cursor_t;

typedef struct font_t{
	Display *dpy;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
	struct font_t *next;
} font_t;

typedef struct{
	unsigned int w, h;
	Display *dpy;
	int screen;
	Window root;
	Drawable drawable;
	GC gc;
	color_t *scheme;
	font_t *fonts;
} drw_t;


/* prototypes */
// drawable abstraction
drw_t *drw_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void drw_resize(drw_t *drw, unsigned int w, unsigned int h);
void drw_free(drw_t *drw);

// font abstraction
unsigned int drw_fontset_getwidth(drw_t *drw, char const *text);

// color scheme abstraction
color_t *drw_scm_create(drw_t *drw, char const *names[], size_t n);

// cursor abstraction
cursor_t *drw_cur_create(drw_t *drw, int shape);
void drw_cur_free(drw_t *drw, cursor_t *cursor);

// drawing context manipulation
void drw_setscheme(drw_t *drw, color_t *scm);

// drawing functions
void drw_rect(drw_t *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert);
int drw_text(drw_t *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, char const *text, int invert);

// map functions
void drw_map(drw_t *drw, Window win, int x, int y, unsigned int w, unsigned int h);


#endif // DRW_H
