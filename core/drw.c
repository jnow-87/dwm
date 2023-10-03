/* See LICENSE file for copyright and license details. */
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <config/config.h>
#include <drw.h>
#include <dwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils.h>

#define UTF_INVALID	0xFFFD
#define UTF_SIZ		4


/* local/static prototypes */
static font_t *xfont_create(drw_t *drw, char const *fontname, FcPattern *fontpattern);
static void xfont_free(font_t *font);
static font_t *drw_fontset_create(drw_t *drw, char const *fonts[], size_t fontcount);
static void drw_fontset_free(font_t *set);

static void drw_color_create(drw_t *drw, color_t *dest, char const *name);

static void drw_setfontset(drw_t *drw, font_t *set);
static unsigned int drw_fontset_getwidth_clamp(drw_t *drw, char const *text, unsigned int n);
static void drw_font_getexts(font_t *font, char const *text, unsigned int len, unsigned int *w, unsigned int *h);

static long utf8decodebyte(char const c, size_t *i);
static size_t utf8validate(long *u, size_t i);
static size_t utf8decode(char const *c, long *u, size_t clen);


/* static variables */
static unsigned char const utfbyte[UTF_SIZ + 1] = {0x80, 0, 0xC0, 0xE0, 0xF0};
static unsigned char const utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static long const utfmin[UTF_SIZ + 1] = {0, 0, 0x80, 0x800, 0x10000};
static long const utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};


/* global functions */
drw_t *drw_create(Display *dpy, int screen, Window root, unsigned int w, unsigned int h){
	drw_t *drw = ecalloc(1, sizeof(drw_t));


	drw->dpy = dpy;
	drw->screen = screen;
	drw->root = root;
	drw->w = w;
	drw->h = h;
	drw->drawable = XCreatePixmap(dpy, root, w, h, DefaultDepth(dpy, screen));
	drw->gc = XCreateGC(dpy, root, 0, NULL);
	XSetLineAttributes(dpy, drw->gc, 1, LineSolid, CapButt, JoinMiter);

	if(!drw_fontset_create(drw, (char const *[]){CONFIG_FONT}, 1))
		die("no fonts could be loaded.");

	dwm.lrpad = drw->fonts->h;
	dwm.statusbar_height = drw->fonts->h + 2;

	return drw;
}

void drw_resize(drw_t *drw, unsigned int w, unsigned int h){
	if(!drw)
		return;

	drw->w = w;
	drw->h = h;
	if(drw->drawable)
		XFreePixmap(drw->dpy, drw->drawable);

	drw->drawable = XCreatePixmap(drw->dpy, drw->root, w, h, DefaultDepth(drw->dpy, drw->screen));
}

void drw_free(drw_t *drw){
	XFreePixmap(drw->dpy, drw->drawable);
	XFreeGC(drw->dpy, drw->gc);
	drw_fontset_free(drw->fonts);
	free(drw);
}

unsigned int drw_fontset_getwidth(drw_t *drw, char const *text){
	if(!drw || !drw->fonts || !text)
		return 0;

	return drw_text(drw, 0, 0, 0, 0, 0, text, 0);
}

color_t *drw_scm_create(drw_t *drw, char const *names[], size_t n){
	/* Wrapper to create color schemes. The caller has to call free(3) on the
	 * returned color scheme when done using it. */
	size_t i;
	color_t *ret;


	/* need at least two colors for a scheme */
	if(!drw || !names || n < 2 || !(ret = ecalloc(n, sizeof(XftColor))))
		return NULL;

	for(i=0; i<n; i++)
		drw_color_create(drw, &ret[i], names[i]);

	return ret;
}

cursor_t *drw_cur_create(drw_t *drw, int shape){
	cursor_t *cur;

	if(!drw || !(cur = ecalloc(1, sizeof(cursor_t))))
		return NULL;

	cur->cursor = XCreateFontCursor(drw->dpy, shape);

	return cur;
}

void drw_cur_free(drw_t *drw, cursor_t *cursor){
	if(!cursor)
		return;

	XFreeCursor(drw->dpy, cursor->cursor);
	free(cursor);
}

void drw_setscheme(drw_t *drw, color_t *scm){
	if(drw)
		drw->scheme = scm;
}

void drw_rect(drw_t *drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert){
	if(!drw || !drw->scheme)
		return;

	XSetForeground(drw->dpy, drw->gc, invert ? drw->scheme[ColBg].pixel : drw->scheme[ColFg].pixel);

	if(filled)	XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
	else		XDrawRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w - 1, h - 1);
}

int drw_text(drw_t *drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, char const *text, int invert){
	int i, ty, ellipsis_x = 0;
	unsigned int tmpw, ew, ellipsis_w = 0, ellipsis_len;
	XftDraw *d = NULL;
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

	if(!drw || (render && (!drw->scheme || !w)) || !text || !drw->fonts)
		return 0;

	if(!render){
		w = invert ? invert : ~invert;
	}
	else{
		XSetForeground(drw->dpy, drw->gc, drw->scheme[invert ? ColFg : ColBg].pixel);
		XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, w, h);
		d = XftDrawCreate(drw->dpy, drw->drawable, DefaultVisual(drw->dpy, drw->screen), DefaultColormap(drw->dpy, drw->screen));
		x += lpad;
		w -= lpad;
	}

	usedfont = drw->fonts;

	if(!ellipsis_width && render)
		ellipsis_width = drw_fontset_getwidth(drw, "...");

	while(1){
		ew = ellipsis_len = utf8strlen = 0;
		utf8str = text;
		nextfont = NULL;

		while(*text){
			utf8charlen = utf8decode(text, &utf8codepoint, UTF_SIZ);

			for(curfont=drw->fonts; curfont; curfont=curfont->next){
				charexists = charexists || XftCharExists(drw->dpy, curfont->xfont, utf8codepoint);

				if(charexists){
					drw_font_getexts(curfont, text, utf8charlen, &tmpw, NULL);

					if(ew + ellipsis_width <= w){
						/* keep track where the ellipsis still fits */
						ellipsis_x = x + ew;
						ellipsis_w = w - ew;
						ellipsis_len = utf8strlen;
					}

					if(ew + tmpw > w){
						overflow = 1;
						/* called from drw_fontset_getwidth_clamp():
						 * it wants the width AFTER the overflow
						 */
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
				XftDrawStringUtf8(d, &drw->scheme[invert ? ColBg : ColFg], usedfont->xfont, x, ty, (XftChar8*)utf8str, utf8strlen);
			}

			x += ew;
			w -= ew;
		}
		if(render && overflow)
			drw_text(drw, ellipsis_x, y, ellipsis_w, h, 0, "...", invert);

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

			if(!drw->fonts->pattern){
				/* Refer to the comment in xfont_create for more information. */
				die("the first font in the cache must be loaded from a font string.");
			}

			fcpattern = FcPatternDuplicate(drw->fonts->pattern);
			FcPatternAddCharSet(fcpattern, FC_CHARSET, fccharset);
			FcPatternAddBool(fcpattern, FC_SCALABLE, FcTrue);

			FcConfigSubstitute(NULL, fcpattern, FcMatchPattern);
			FcDefaultSubstitute(fcpattern);
			match = XftFontMatch(drw->dpy, drw->screen, fcpattern, &result);

			FcCharSetDestroy(fccharset);
			FcPatternDestroy(fcpattern);

			if(match){
				usedfont = xfont_create(drw, NULL, match);

				if(usedfont && XftCharExists(drw->dpy, usedfont->xfont, utf8codepoint)){
					for(curfont=drw->fonts; curfont->next; curfont=curfont->next); /* NOP */
					curfont->next = usedfont;
				}
				else{
					xfont_free(usedfont);
					nomatches.codepoint[++nomatches.idx % nomatches_len] = utf8codepoint;
no_match:
					usedfont = drw->fonts;
				}
			}
		}
	}

	if(d)
		XftDrawDestroy(d);

	return x + (render ? w : 0);
}

void drw_map(drw_t *drw, Window win, int x, int y, unsigned int w, unsigned int h){
	if(!drw)
		return;

	XCopyArea(drw->dpy, drw->drawable, win, drw->gc, x, y, w, h, x, y);
	XSync(drw->dpy, False);
}


/* local functions */
static long utf8decodebyte(char const c, size_t *i){
	for(*i=0; *i<(UTF_SIZ + 1); ++(*i)){
		if(((unsigned char)c & utfmask[*i]) == utfbyte[*i])
			return (unsigned char)c & ~utfmask[*i];
	}

	return 0;
}

static size_t utf8validate(long *u, size_t i){
	if(!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;

	for(i=1; *u>utfmax[i]; ++i);

	return i;
}

static size_t utf8decode(char const *c, long *u, size_t clen){
	size_t i, j, len, type;
	long udecoded;


	*u = UTF_INVALID;

	if(!clen)
		return 0;

	udecoded = utf8decodebyte(c[0], &len);

	if(!BETWEEN(len, 1, UTF_SIZ))
		return 1;

	for(i=1, j=1; i<clen && j<len; ++i, ++j){
		udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);

		if(type)
			return j;
	}

	if(j < len)
		return 0;

	*u = udecoded;
	utf8validate(u, len);

	return len;
}


/* local functions */
static font_t *xfont_create(drw_t *drw, char const *fontname, FcPattern *fontpattern){
	/* This function is an implementation detail. Library users should use
	 * drw_fontset_create instead.
	 */
	font_t *font;
	XftFont *xfont = NULL;
	FcPattern *pattern = NULL;


	if(fontname){
		/* Using the pattern found at font->xfont->pattern does not yield the
		 * same substitution results as using the pattern returned by
		 * FcNameParse; using the latter results in the desired fallback
		 * behaviour whereas the former just results in missing-character
		 * rectangles being drawn, at least with some fonts. */
		if(!(xfont = XftFontOpenName(drw->dpy, drw->screen, fontname))){
			fprintf(stderr, "error, cannot load font from name: '%s'\n", fontname);

			return NULL;
		}

		if(!(pattern = FcNameParse((FcChar8*)fontname))){
			fprintf(stderr, "error, cannot parse font name to pattern: '%s'\n", fontname);
			XftFontClose(drw->dpy, xfont);

			return NULL;
		}
	}
	else if(fontpattern){
		if(!(xfont = XftFontOpenPattern(drw->dpy, fontpattern))){
			fprintf(stderr, "error, cannot load font from pattern.\n");

			return NULL;
		}
	}
	else
		die("no font specified.");

	font = ecalloc(1, sizeof(font_t));
	font->xfont = xfont;
	font->pattern = pattern;
	font->h = xfont->ascent + xfont->descent;
	font->dpy = drw->dpy;

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

static font_t *drw_fontset_create(drw_t *drw, char const *fonts[], size_t fontcount){
	font_t *cur, *ret = NULL;
	size_t i;


	if(!drw || !fonts)
		return NULL;

	for(i=1; i<=fontcount; i++){
		if((cur = xfont_create(drw, fonts[fontcount - i], NULL))){
			cur->next = ret;
			ret = cur;
		}
	}

	return (drw->fonts = ret);
}

static void drw_fontset_free(font_t *font){
	if(font){
		drw_fontset_free(font->next);
		xfont_free(font);
	}
}

static void drw_color_create(drw_t *drw, color_t *dest, char const *name){
	if(!drw || !dest || !name)
		return;

	if(!XftColorAllocName(drw->dpy, DefaultVisual(drw->dpy, drw->screen), DefaultColormap(drw->dpy, drw->screen), name, dest))
		die("error, cannot allocate color '%s'", name);
}

static void drw_setfontset(drw_t *drw, font_t *set){
	if(drw)
		drw->fonts = set;
}

static unsigned int drw_fontset_getwidth_clamp(drw_t *drw, char const *text, unsigned int n){
	unsigned int tmp = 0;


	if(drw && drw->fonts && text && n)
		tmp = drw_text(drw, 0, 0, 0, 0, 0, text, n);

	return MIN(n, tmp);
}

static void drw_font_getexts(font_t *font, char const *text, unsigned int len, unsigned int *w, unsigned int *h){
	XGlyphInfo ext;


	if(!font || !text)
		return;

	XftTextExtentsUtf8(font->dpy, font->xfont, (XftChar8*)text, len, &ext);

	if(w)
		*w = ext.xOff;

	if(h)
		*h = font->h;
}
