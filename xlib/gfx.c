/* See LICENSE file for copyright and license details. */
#include <stdlib.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <config/config.h>
#include <xlib/gfx.h>
#include <core/dwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/math.h>
#include <utils/list.h>

#define UTF_INVALID	0xFFFD
#define UTF_SIZ		4


/* local/static prototypes */
static font_t *xfont_create(gfx_t *gfx, char const *fontname, FcPattern *fontpattern);
static void xfont_free(font_t *font);

static void color_create(gfx_t *gfx, color_t *dest, char const *name);

static void font_extents(font_t *font, char const *text, unsigned int len, unsigned int *w, unsigned int *h);

static size_t utf8_decode(char const *c, long *u, size_t clen);
static long utf8_decode_byte(char const c, size_t *i);
static size_t utf8_validate(long *u, size_t i);


/* static variables */
static unsigned char const utfbyte[UTF_SIZ + 1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
static unsigned char const utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static long const utfmin[UTF_SIZ + 1] = {0, 0, 0x80, 0x800, 0x10000};
static long const utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};


/* global functions */
gfx_t *gfx_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h){
	gfx_t *gfx;
	font_t *f;


	gfx = calloc(1, sizeof(gfx_t));

	if(gfx == 0x0)
		return 0x0;

	gfx->dpy = dpy;
	gfx->screen = screen;
	gfx->root = root;
	gfx->w = w;
	gfx->h = h;
	gfx->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	gfx->gc = XCreateGC(dpy, root, 0, 0x0);
	XSetLineAttributes(dpy, gfx->gc, 1, LineSolid, CapButt, JoinMiter);
	f = xfont_create(gfx, CONFIG_FONT, 0x0);

	if(f == 0x0)
		die("no fonts could be loaded.");

	gfx->fonts = 0x0;
	list_add_tail(gfx->fonts, f);

	dwm.lrpad = f->h;

	return gfx;
}

void gfx_resize(gfx_t *gfx, unsigned int w, unsigned int h){
	if(!gfx)
		return;

	gfx->w = w;
	gfx->h = h;
	if(gfx->drawable)
		XFreePixmap(gfx->dpy, gfx->drawable);

	gfx->drawable = XCreatePixmap(gfx->dpy, gfx->root, w, h, DefaultDepth(gfx->dpy, gfx->screen));
}

void gfx_free(gfx_t *gfx){
	font_t *f;


	XFreePixmap(gfx->dpy, gfx->drawable);
	XFreeGC(gfx->dpy, gfx->gc);

	list_for_each(gfx->fonts, f){
		list_rm(gfx->fonts, f);
		xfont_free(f);
	}

	free(gfx);
}

unsigned int gfx_fontset_getwidth(gfx_t *gfx, char const *text){
	if(!gfx || !gfx->fonts || !text)
		return 0;

	return gfx_text(gfx, 0, 0, 0, 0, 0, text, 0);
}

color_t *gfx_scm_create(gfx_t *gfx, char const *names[], size_t n){
	/* Wrapper to create color schemes. The caller has to call free(3) on the
	 * returned color scheme when done using it. */
	size_t i;
	color_t *c;


	/* need at least two colors for a scheme */
	if(n < 2)
		return 0x0;

	c = calloc(n, sizeof(XftColor));

	if(c == 0x0)
		return 0x0;

	for(i=0; i<n; i++)
		color_create(gfx, &c[i], names[i]);

	return c;
}

cursor_t *gfx_cur_create(gfx_t *gfx, int shape){
	cursor_t *c;


	c = calloc(1, sizeof(cursor_t));

	if(c == 0x0)
		return 0x0;

	c->cursor = XCreateFontCursor(gfx->dpy, shape);

	return c;
}

void gfx_cur_free(gfx_t *gfx, cursor_t *cursor){
	if(!cursor)
		return;

	XFreeCursor(gfx->dpy, cursor->cursor);
	free(cursor);
}

void gfx_setscheme(gfx_t *gfx, color_t *scm){
	if(gfx)
		gfx->scheme = scm;
}

void gfx_rect(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, int filled, int invert){
	if(!gfx || !gfx->scheme)
		return;

	XSetForeground(gfx->dpy, gfx->gc, invert ? gfx->scheme[ColBg].pixel : gfx->scheme[ColFg].pixel);

	if(filled)	XFillRectangle(gfx->dpy, gfx->drawable, gfx->gc, x, y, w, h);
	else		XDrawRectangle(gfx->dpy, gfx->drawable, gfx->gc, x, y, w - 1, h - 1);
}

int gfx_text(gfx_t *gfx, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, char const *text, int invert){
	int i, ty, ellipsis_x = 0;
	unsigned int tmpw, ew, ellipsis_w = 0, ellipsis_len;
	XftDraw *d = 0x0;
	font_t *usedfont, *curfont, *nextfont;
	int utf8strlen, utf8charlen, render = x || y || w || h;
	long utf8codepoint = 0;
	char const *utf8str;
	FcCharSet *fccharset;
	FcPattern *fcpattern;
	FcPattern *match;
	XftResult result;
	int charexists = 0, overflow = 0;


	/* keep track of a couple codepoints for which we have no match. */
	enum{
		nomatches_len = 64
	};

	static struct{
		long codepoint[nomatches_len];
		unsigned int idx;
	} nomatches;

	static unsigned int ellipsis_width = 0;

	if(!gfx || (render && (!gfx->scheme || !w)) || !text || !gfx->fonts)
		return 0;

	if(!render){
		w = invert ? invert : ~invert;
	}
	else{
		XSetForeground(gfx->dpy, gfx->gc, gfx->scheme[invert ? ColFg : ColBg].pixel);
		XFillRectangle(gfx->dpy, gfx->drawable, gfx->gc, x, y, w, h);
		d = XftDrawCreate(gfx->dpy, gfx->drawable, DefaultVisual(gfx->dpy, gfx->screen), DefaultColormap(gfx->dpy, gfx->screen));
		x += lpad;
		w -= lpad;
	}

	usedfont = gfx->fonts;

	if(!ellipsis_width && render)
		ellipsis_width = gfx_fontset_getwidth(gfx, "...");

	while(1){
		ew = ellipsis_len = utf8strlen = 0;
		utf8str = text;
		nextfont = 0x0;

		while(*text){
			utf8charlen = utf8_decode(text, &utf8codepoint, UTF_SIZ);

			list_for_each(gfx->fonts, curfont){
				charexists = charexists || XftCharExists(gfx->dpy, curfont->xfont, utf8codepoint);

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
				ty = y + (h - usedfont->h) / 2 + usedfont->xfont->ascent;
				XftDrawStringUtf8(d, &gfx->scheme[invert ? ColBg : ColFg], usedfont->xfont, x, ty, (XftChar8*)utf8str, utf8strlen);
			}

			x += ew;
			w -= ew;
		}
		if(render && overflow)
			gfx_text(gfx, ellipsis_x, y, ellipsis_w, h, 0, "...", invert);

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

			for(i=0; i<nomatches_len; ++i){
				/* avoid calling XftFontMatch if we know we won't find a match */
				if(utf8codepoint == nomatches.codepoint[i])
					goto no_match;
			}

			fccharset = FcCharSetCreate();
			FcCharSetAddChar(fccharset, utf8codepoint);

			if(!gfx->fonts->pattern){
				/* Refer to the comment in xfont_create for more information. */
				die("the first font in the cache must be loaded from a font string.");
			}

			fcpattern = FcPatternDuplicate(gfx->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);

			FcConfigSubstitute(0x0, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(gfx->dpy, gfx->screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if(match){
				usedfont = xfont_create(gfx, 0x0, match);

				if(usedfont && XftCharExists(gfx->dpy, usedfont->xfont, utf8codepoint)){
					list_add_tail(gfx->fonts, usedfont);
				}
				else{
					xfont_free(usedfont);
					nomatches.codepoint[++nomatches.idx % nomatches_len] = utf8codepoint;
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
	if(!gfx)
		return;

	XCopyArea(gfx->dpy, gfx->drawable, win, gfx->gc, x, y, w, h, x, y);
	XSync(gfx->dpy, False);
}


/* local functions */
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

static font_t *xfont_create(gfx_t *gfx, char const *fontname, FcPattern *fontpattern){
	font_t *font;
	XftFont *xfont = 0x0;
	FcPattern *pattern = 0x0;


	if(fontname){
		/* Using the pattern found at font->xfont->pattern does not yield the
		 * same substitution results as using the pattern returned by
		 * FcNameParse; using the latter results in the desired fallback
		 * behaviour whereas the former just results in missing-character
		 * rectangles being drawn, at least with some fonts. */
		if(!(xfont = XftFontOpenName(gfx->dpy, gfx->screen, fontname))){
			fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);

			return 0x0;
		}

		if(!(pattern = FcNameParse((FcChar8*)fontname))){
			fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
			XftFontClose(gfx->dpy, xfont);

			return 0x0;
		}
	}
	else if(fontpattern){
		if(!(xfont = XftFontOpenPattern(gfx->dpy, fontpattern))){
			fprintf(stderr, "error, cannot load font from pattern.\n");

			return 0x0;
		}
	}
	else
		die("no font specified.");

	font = calloc(1, sizeof(font_t));

	if(font == 0x0)
		return 0x0;

	font->xfont = xfont;
	font->pattern = pattern;
	font->h = xfont->ascent + xfont->descent;
	font->dpy = gfx->dpy;

	return font;
}

static void xfont_free(font_t *font){
	if(!font)
		return;

	if(font->pattern)
		FcPatternDestroy(font->pattern);

	XftFontClose(font->dpy, font->xfont);
	free(font);
}

static void color_create(gfx_t *gfx, color_t *dest, char const *name){
	if(!gfx || !dest || !name)
		return;

	if(!XftColorAllocName(gfx->dpy, DefaultVisual(gfx->dpy, gfx->screen), DefaultColormap(gfx->dpy, gfx->screen), name, dest))
		die("error, cannot allocate color '%s'", name);
}

static void font_extents(font_t *font, char const *text, unsigned int len, unsigned int *w, unsigned int *h){
	XGlyphInfo ext;


	if(!font || !text)
		return;

	XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8*)text, len, &ext);

	if(w)
		*w = ext.xOff;

	if(h)
		*h = font->h;
}
