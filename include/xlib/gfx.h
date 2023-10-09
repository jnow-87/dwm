/* See LICENSE file for copyright and license details. */

#ifndef GFX_H
#define GFX_H


#include <stddef.h>
#include <fontconfig/fontconfig.h>
#include <X11/X.h>
#include <X11/Xft/Xft.h>


/* types */
typedef enum{
	SCOL_FB,
	SCOL_BB,
	SCOL_BORDER,
	NSCOLORS
} scheme_index_t;

typedef enum{
	SCM_NORM,
	SCM_FOCUS,
	SCM_SPACER,
	NSCMS
} scheme_t;

typedef enum{
	CUR_NONE = -1,
	CUR_NORM,
	CUR_RESIZE,
	CUR_MOVE,
	NCURSORS
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
	cursor_t cursors[NCURSORS];
	color_t **scheme;
	size_t nscheme;
} gfx_t;


/* prototypes */
gfx_t *gfx_create(unsigned int w, unsigned int h);
void gfx_free(gfx_t *gfx);
void gfx_resize(gfx_t *gfx, unsigned int w, unsigned int h);

size_t gfx_text_width(gfx_t *gfx, char const *text);

void gfx_rect(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, scheme_t scheme, int filled, int invert);
int gfx_text(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, scheme_t scheme, unsigned int lpad, char const *text, int invert);

void gfx_map(gfx_t *gfx, Window win, int x, int y, unsigned int w, unsigned int h);


#endif // GFX_H
