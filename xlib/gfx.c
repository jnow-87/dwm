/* See LICENSE file for copyright and license details. */
#include <config/config.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <core/dwm.h>
#include <core/scheme.h>
#include <xlib/gfx.h>
#include <xlib/xlib.h>
#include <utils/list.h>
#include <utils/log.h>
#include <utils/utils.h>


/* macros */
#define NOMATCHES_LEN	64

#define UTF_INVALID	0xFFFD
#define UTF_SIZ		4

#define BETWEEN(X, A, B)	((A) <= (X) && (X) <= (B))


/* local/static prototypes */
static font_t *font_create_from_name(gfx_t *gfx, char const *name);
static font_t *font_create_from_pattern(gfx_t *gfx, FcPattern *pattern);
static void font_free(font_t *font);
static void font_extents(font_t *font, char const *text, unsigned int len, unsigned int *w, unsigned int *h);
static unsigned int font_height(font_t *font);

static cursor_t cursor_create(gfx_t *gfx, int shape);
static void cursor_free(gfx_t *gfx, cursor_t cursor);

static int scheme_create(gfx_t *gfx, color_scheme_t *scheme, scheme_t *scm);
static void scheme_destroy(gfx_t *gfx, scheme_t *scheme);

static size_t utf8_decode(char const *c, long *u, size_t clen);
static long utf8_decode_byte(char const c, size_t *i);
static size_t utf8_validate(long *u, size_t i);


/* static variables */
static unsigned char const utfbyte[UTF_SIZ + 1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
static unsigned char const utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static long const utfmin[UTF_SIZ + 1] = {0, 0, 0x80, 0x800, 0x10000};
static long const utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};


/* global functions */
gfx_t *gfx_create(unsigned int w, unsigned int h){
	gfx_t *gfx;
	font_t *font;
	color_scheme_t *scheme;


	/* init gfx */
	gfx = calloc(1, sizeof(gfx_t));

	if(gfx == 0x0)
		goto err_0;

	gfx->drawable = XCreatePixmap(dwm.dpy, dwm.root, w, h, DefaultDepth(dwm.dpy, dwm.screen));
	gfx->gc = XCreateGC(dwm.dpy, dwm.root, 0, 0x0);
	XSetLineAttributes(dwm.dpy, gfx->gc, 1, LineSolid, CapButt, JoinMiter);

	/* init fonts */
	font = font_create_from_name(gfx, CONFIG_FONT);

	if(font == 0x0)
		goto err_1;

	list_add_tail(gfx->fonts, font);

	/* init color schemes */
	if(__stop_schemes - __start_schemes > NSCMS)
		INFO("more than %d schemes defined, the rest will be ignored\n", NSCMS);

	config_for_each(schemes, scheme){
		if(scheme_create(gfx, scheme, gfx->schemes + scheme->id) != 0)
			goto err_1;
	}

	/* init cursors */
	gfx->cursors[CUR_NORM] = cursor_create(dwm.gfx, XC_left_ptr);
	gfx->cursors[CUR_RESIZE] = cursor_create(dwm.gfx, XC_sizing);
	gfx->cursors[CUR_MOVE] = cursor_create(dwm.gfx, XC_fleur);

	return gfx;


err_1:
	gfx_free(gfx);

err_0:
	return 0x0;
}

void gfx_free(gfx_t *gfx){
	font_t *font;


	for(size_t i=0; i<NCURSORS; i++)
		cursor_free(dwm.gfx, gfx->cursors[i]);

	for(size_t i=0; i<NSCMS; i++)
		scheme_destroy(gfx, gfx->schemes + i);

	list_for_each(gfx->fonts, font){
		list_rm(gfx->fonts, font);
		font_free(font);
	}

	XFreePixmap(dwm.dpy, gfx->drawable);
	XFreeGC(dwm.dpy, gfx->gc);

	free(gfx);
}

void gfx_resize(gfx_t *gfx, unsigned int w, unsigned int h){
	if(gfx->drawable)
		XFreePixmap(dwm.dpy, gfx->drawable);

	gfx->drawable = XCreatePixmap(dwm.dpy, dwm.root, w, h, DefaultDepth(dwm.dpy, dwm.screen));
}

size_t gfx_text_width(gfx_t *gfx, char const *text){
	return gfx_text(gfx, 0, 0, 0, 0, SCM_NORM, 0, text, 0);
}

void gfx_rect(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, scheme_id_t scheme, int filled, int invert){
	scheme_t *scm = gfx->schemes + scheme;


	XSetForeground(dwm.dpy, gfx->gc, invert ? scm->bg.pixel : scm->fg.pixel);

	if(filled)	XFillRectangle(dwm.dpy, gfx->drawable, gfx->gc, x, y, w, h);
	else		XDrawRectangle(dwm.dpy, gfx->drawable, gfx->gc, x, y, w - 1, h - 1);
}

int gfx_text(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, scheme_id_t scheme, unsigned int lpad, char const *text, int invert){
	int i, ty, ellipsis_x = 0;
	unsigned int tmpw, ew, ellipsis_w = 0, ellipsis_len;
	int utf8strlen, utf8charlen, render = x || y || w || h;
	long utf8codepoint = 0;
	XftDraw *d = 0x0;
	scheme_t *scm = gfx->schemes + scheme;
	font_t *usedfont, *curfont, *nextfont;
	char const *utf8str;
	FcCharSet *fccharset;
	FcPattern *fcpattern;
	FcPattern *match;
	XftResult result;
	int charexists = 0, overflow = 0;


	static struct{
		long codepoint[NOMATCHES_LEN];
		unsigned int idx;
	} nomatches;

	static size_t ellipsis_width = 0;

	if(render && !w)
		return 0;

	if(!render){
		w = invert ? invert : ~invert;
	}
	else{
		XSetForeground(dwm.dpy, gfx->gc, invert ? scm->fg.pixel : scm->bg.pixel);
		XFillRectangle(dwm.dpy, gfx->drawable, gfx->gc, x, y, w, h);
		d = XftDrawCreate(dwm.dpy, gfx->drawable, DefaultVisual(dwm.dpy, dwm.screen), DefaultColormap(dwm.dpy, dwm.screen));
		x += lpad;
		w -= lpad;
	}

	usedfont = gfx->fonts;

	if(!ellipsis_width && render)
		ellipsis_width = gfx_text_width(gfx, "...");

	while(1){
		ew = ellipsis_len = utf8strlen = 0;
		utf8str = text;
		nextfont = 0x0;

		while(*text){
			utf8charlen = utf8_decode(text, &utf8codepoint, UTF_SIZ);

			list_for_each(gfx->fonts, curfont){
				charexists = charexists || XftCharExists(dwm.dpy, curfont->xfont, utf8codepoint);

				if(charexists){
					font_extents(curfont, text, utf8charlen, &tmpw, 0x0);

					if(ew + ellipsis_width <= w){
						/* keep track where the ellipsis still fits */
						ellipsis_x = x + ew;
						ellipsis_w = w - ew;
						ellipsis_len = utf8strlen;
					}

					if(ew + tmpw > w){
						overflow = 1;

						if(!render)	x += tmpw;
						else		utf8strlen = ellipsis_len;
					}
					else if(curfont == usedfont){
						utf8strlen += utf8charlen;
						text += utf8charlen;
						ew += tmpw;
					}
					else
						nextfont = curfont;

					break;
				}
			}

			if(overflow || !charexists || nextfont)
				break;

			charexists = 0;
		}

		if(utf8strlen){
			if(render){
				ty = y + (h - font_height(usedfont)) / 2 + usedfont->xfont->ascent;
				XftDrawStringUtf8(d, invert ? &scm->bg : &scm->fg, usedfont->xfont, x, ty, (XftChar8*)utf8str, utf8strlen);
			}

			x += ew;
			w -= ew;
		}
		if(render && overflow)
			gfx_text(gfx, ellipsis_x, y, ellipsis_w, h, scheme, 0, "...", invert);

		if(!*text || overflow)
			break;

		if(nextfont){
			charexists = 0;
			usedfont = nextfont;
		}
		else{
			/* Regardless of whether or not a fallback font is found, the
			 * character must be drawn. */
			charexists = 1;

			for(i=0; i<NOMATCHES_LEN; ++i){
				/* avoid calling XftFontMatch if we know we won't find a match */
				if(utf8codepoint == nomatches.codepoint[i])
					goto no_match;
			}

			fccharset = FcCharSetCreate();
			FcCharSetAddChar(fccharset, utf8codepoint);

			if(!gfx->fonts->pattern){
				/* Refer to the comment in xfont_create for more information. */
				EEXIT("the first font in the cache must be loaded from a font string\n");
			}

			fcpattern = FcPatternDuplicate(gfx->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);

			FcConfigSubstitute(0x0, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(dwm.dpy, dwm.screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if(match){
				usedfont = font_create_from_pattern(gfx, match);

				if(usedfont && XftCharExists(dwm.dpy, usedfont->xfont, utf8codepoint)){
					list_add_tail(gfx->fonts, usedfont);
				}
				else{
					font_free(usedfont);
					nomatches.codepoint[++nomatches.idx % NOMATCHES_LEN] = utf8codepoint;
no_match:
					usedfont = gfx->fonts;
				}
			}
		}
	}

	if(d)
		XftDrawDestroy(d);

	return x + (render ? w : 0);
}

void gfx_map(gfx_t *gfx, Window win, int x, int y, unsigned int w, unsigned int h){
	XCopyArea(dwm.dpy, gfx->drawable, win, gfx->gc, x, y, w, h, x, y);
	xlib_sync();
}


/* local functions */
static font_t *font_create_from_name(gfx_t *gfx, char const *name){
	font_t *font;


	font = malloc(sizeof(font_t));

	if(font == 0x0)
		goto err_0;

	/* Using the pattern found at font->xfont->pattern does not yield the
	 * same substitution results as using the pattern returned by
	 * FcNameParse; using the latter results in the desired fallback
	 * behaviour whereas the former just results in missing-character
	 * rectangles being drawn, at least with some fonts. */
	font->xfont = XftFontOpenName(dwm.dpy, dwm.screen, name);

	if(font->xfont == 0x0)
		goto err_1;

	font->pattern = FcNameParse((FcChar8*)name);

	if(font->pattern == 0x0)
		goto err_1;

	return font;


err_1:
	ERROR("parsing font name to pattern\n");
	font_free(font);

err_0:
	ERROR("loading font %s\n", name);

	return font;
}

static font_t *font_create_from_pattern(gfx_t *gfx, FcPattern *pattern){
	font_t *font;


	font = malloc(sizeof(font_t));

	if(font == 0x0)
		goto err_0;

	font->pattern = 0x0;
	font->xfont = XftFontOpenPattern(dwm.dpy, pattern);

	if(font->xfont == 0x0)
		goto err_1;

	return font;


err_1:
	ERROR("parsing font name to pattern\n");
	font_free(font);

err_0:
	ERROR("loading font from pattern\n");

	return 0x0;
}

static void font_free(font_t *font){
	if(font->pattern != 0x0)
		FcPatternDestroy(font->pattern);

	if(font->xfont != 0x0)
		XftFontClose(dwm.dpy, font->xfont);

	free(font);
}

static void font_extents(font_t *font, char const *text, unsigned int len, unsigned int *w, unsigned int *h){
	XGlyphInfo ext;


	XftTextExtentsUtf8(dwm.dpy, font->xfont, (XftChar8*)text, len, &ext);

	if(w)
		*w = ext.xOff;

	if(h)
		*h = font_height(font);
}

static unsigned int font_height(font_t *font){
	return font->xfont->ascent + font->xfont->descent;
}

cursor_t cursor_create(gfx_t *gfx, int shape){
	return XCreateFontCursor(dwm.dpy, shape);
}

void cursor_free(gfx_t *gfx, cursor_t cursor){
	XFreeCursor(dwm.dpy, cursor);
}

static int scheme_create(gfx_t *gfx, color_scheme_t *scheme, scheme_t *scm){
	char const *names[] = { scheme->fg, scheme->bg, scheme->border };
	color_t *colors[] = { &scm->fg, &scm->bg, &scm->border };
	Visual *visual = DefaultVisual(dwm.dpy, dwm.screen);
	Colormap colmap = DefaultColormap(dwm.dpy, dwm.screen);


	for(size_t i=0; i<LENGTH(colors); i++){
		if(!XftColorAllocName(dwm.dpy, visual, colmap, names[i], colors[i])){
			for(; i>0; i--)
				XftColorFree(dwm.dpy, visual, colmap, colors[i - 1]);

			return -1;
		}
	}

	return 0;
}

static void scheme_destroy(gfx_t *gfx, scheme_t *scheme){
	color_t *colors[] = { &scheme->fg, &scheme->bg, &scheme->border };


	for(size_t i=0; i<LENGTH(colors); i++)
		XftColorFree(dwm.dpy, DefaultVisual(dwm.dpy, dwm.screen), DefaultColormap(dwm.dpy, dwm.screen), colors[i]);
}

static size_t utf8_decode(char const *c, long *u, size_t clen){
	size_t i, j, len, type;
	long udecoded;


	*u = UTF_INVALID;

	if(!clen)
		return 0;

	udecoded = utf8_decode_byte(c[0], &len);

	if(!BETWEEN(len, 1, UTF_SIZ))
		return 1;

	for(i=1, j=1; i<clen && j<len; ++i, ++j){
		udecoded = (udecoded << 6) | utf8_decode_byte(c[i], &type);

		if(type)
			return j;
	}

	if(j < len)
		return 0;

	*u = udecoded;
	utf8_validate(u, len);

	return len;
}

static long utf8_decode_byte(char const c, size_t *i){
	for(*i=0; *i<(UTF_SIZ + 1); ++(*i)){
		if(((unsigned char)c & utfmask[*i]) == utfbyte[*i])
			return (unsigned char)c & ~utfmask[*i];
	}

	return 0;
}

static size_t utf8_validate(long *u, size_t i){
	if(!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;

	for(i=1; *u>utfmax[i]; ++i);

	return i;
}
