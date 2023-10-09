/* See LICENSE file for copyright and license details. */

#ifndef GFX_H
#define GFX_H


#include <fontconfig/fontconfig.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>


/* macros */
#define TEXTW(X)	(gfx_fontset_getwidth(dwm.gfx, (X)) + dwm.lrpad)


/* types */
// TODO rename
typedef enum{
	ColFg,
	ColBg,
	ColBorder,
	ColLast
} scheme_index_t;

typedef enum{
	SchemeNorm,
	SchemeSel,
	SchemeSpacer,
} scheme_t;

// TODO rename
typedef enum{
	CurNone = -1,
	CurNormal,
	CurResize,
	CurMove,
	CurLast
} cursor_type_t;

typedef XftColor color_t;
typedef Cursor cursor_t;

typedef struct font_t{
	struct font_t *prev,
				  *next;

	XftFont *xfont;
	FcPattern *pattern;
} font_t;

typedef struct{
	Drawable drawable;
	GC gc;

	font_t *fonts;
	cursor_t cursors[CurLast];
	color_t **scheme;
	size_t nscheme;
} gfx_t;


/* prototypes */
// drawable abstraction
gfx_t *gfx_create(unsigned int w, unsigned int h, char const *scheme[][ColLast], size_t n);
void gfx_free(gfx_t *gfx);
void gfx_resize(gfx_t *gfx, unsigned int w, unsigned int h);

unsigned int gfx_fontset_getwidth(gfx_t *gfx, char const *text);

void gfx_rect(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, scheme_t scheme, int filled, int invert);
int gfx_text(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, scheme_t scheme, unsigned int lpad, char const *text, int invert);

void gfx_map(gfx_t *gfx, Window win, int x, int y, unsigned int w, unsigned int h);


#endif // GFX_H
