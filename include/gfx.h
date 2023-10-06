/* See LICENSE file for copyright and license details. */

#ifndef gfx_H
#define gfx_H


#include <X11/X.h>
#include <X11/Xft/Xft.h>


/* macros */
#define TEXTW(X)	(gfx_fontset_getwidth(dwm.gfx, (X)) + dwm.lrpad)


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
	struct font_t *prev,
				  *next;

	Display *dpy;
	unsigned int h;
	XftFont *xfont;
	FcPattern *pattern;
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
} gfx_t;


/* prototypes */
// drawable abstraction
gfx_t *gfx_create(Display *dpy, int screen, Window win, unsigned int w, unsigned int h);
void gfx_resize(gfx_t *gfx, unsigned int w, unsigned int h);
void gfx_free(gfx_t *gfx);

// font abstraction
unsigned int gfx_fontset_getwidth(gfx_t *gfx, char const *text);

// color scheme abstraction
color_t *gfx_scm_create(gfx_t *gfx, char const *names[], size_t n);

// cursor abstraction
cursor_t *gfx_cur_create(gfx_t *gfx, int shape);
void gfx_cur_free(gfx_t *gfx, cursor_t *cursor);

// drawing context manipulation
void gfx_setscheme(gfx_t *gfx, color_t *scm);

// drawing functions
void gfx_rect(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, int filled, int invert);
int gfx_text(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, char const *text, int invert);

// map functions
void gfx_map(gfx_t *gfx, Window win, int x, int y, unsigned int w, unsigned int h);


#endif // gfx_H
