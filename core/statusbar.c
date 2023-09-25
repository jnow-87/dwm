#include <config/config.h>
#include <X11/Xatom.h>
#include <client.h>
#include <colors.h>
#include <dwm.h>
#include <config.h>


/* local/static prototypes */
static int gettextprop(Window w, Atom atom, char *text, unsigned int size);


/* global variables */
char stext[256];


/* global functions */
void drawbar(monitor_t *m){
	int x, w, tw = 0;
	int boxs = dwm.drw->fonts->h / 9;
	int boxw = dwm.drw->fonts->h / 6 + 2;
	unsigned int i, occ = 0, urg = 0;
	client_t *c;


	if(!m->showbar)
		return;

	/* draw status first so it can be overdrawn by tags later */
	if(m == dwm.selmon){				   /* status is only drawn on selected monitor */
		drw_setscheme(dwm.drw, dwm.scheme[SchemeNorm]);
		tw = TEXTW(stext) - dwm.lrpad + 2; /* 2px right padding */
		drw_text(dwm.drw, m->ww - tw, 0, tw, dwm.statusbar_height, 0, stext, 0);
	}

	for(c=m->clients; c; c=c->next){
		occ |= c->tags;
		if(c->isurgent)
			urg |= c->tags;
	}

	x = 0;

	for(i=0; i<ntags; i++){
		w = TEXTW(tags[i]);
		drw_setscheme(dwm.drw, dwm.scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
		drw_text(dwm.drw, x, 0, w, dwm.statusbar_height, dwm.lrpad / 2, tags[i], urg & 1 << i);

		if(occ & 1 << i)
			drw_rect(dwm.drw, x + boxs, boxs, boxw, boxw, m == dwm.selmon && dwm.selmon->sel && dwm.selmon->sel->tags & 1 << i, urg & 1 << i);

		x += w;
	}

	w = TEXTW(m->ltsymbol);
	drw_setscheme(dwm.drw, dwm.scheme[SchemeNorm]);
	x = drw_text(dwm.drw, x, 0, w, dwm.statusbar_height, dwm.lrpad / 2, m->ltsymbol, 0);

	if((w = m->ww - tw - x) > dwm.statusbar_height){
		if(m->sel){
			drw_setscheme(dwm.drw, dwm.scheme[m == dwm.selmon ? SchemeSel : SchemeNorm]);
			drw_text(dwm.drw, x, 0, w, dwm.statusbar_height, dwm.lrpad / 2, m->sel->name, 0);

			if(m->sel->isfloating)
				drw_rect(dwm.drw, x + boxs, boxs, boxw, boxw, m->sel->isfixed, 0);
		}
		else{
			drw_setscheme(dwm.drw, dwm.scheme[SchemeNorm]);
			drw_rect(dwm.drw, x, 0, w, dwm.statusbar_height, 1, 1);
		}
	}

	drw_map(dwm.drw, m->barwin, 0, 0, m->ww, dwm.statusbar_height);
}

void drawbars(void){
	monitor_t *m;


	for(m=dwm.mons; m; m=m->next)
		drawbar(m);
}

void updatebars(void){
	XSetWindowAttributes wa = {
		.override_redirect = True,
		.background_pixmap = ParentRelative,
		.event_mask = ButtonPressMask | ExposureMask
	};
	XClassHint ch = {"dwm", "dwm"};
	monitor_t *m;


	for(m=dwm.mons; m; m=m->next){
		if(m->barwin)
			continue;

		m->barwin = XCreateWindow(dwm.dpy, dwm.root, m->wx, m->by, m->ww, dwm.statusbar_height, 0, DefaultDepth(dwm.dpy, dwm.screen), CopyFromParent, DefaultVisual(dwm.dpy, dwm.screen), CWOverrideRedirect | CWBackPixmap | CWEventMask, &wa);
		XDefineCursor(dwm.dpy, m->barwin, dwm.cursor[CurNormal]->cursor);
		XMapRaised(dwm.dpy, m->barwin);
		XSetClassHint(dwm.dpy, m->barwin, &ch);
	}
}

void updatebarpos(monitor_t *m){
	m->wy = m->my;
	m->wh = m->mh;

	if(m->showbar){
		m->wh -= dwm.statusbar_height;

		if(CONFIG_STATUSBAR_TOP){
			m->by = m->wy;
			m->wy += dwm.statusbar_height;
		}
		else
			m->by = m->wy + m->wh;
	}
	else
		m->by = -dwm.statusbar_height;
}

void updatestatus(void){
	gettextprop(dwm.root, XA_WM_NAME, stext, sizeof(stext));
	drawbar(dwm.selmon);
}

void updatetitle(client_t *c){
	if(!gettextprop(c->win, dwm.netatom[NetWMName], c->name, sizeof c->name))
		gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
}


/* local functions */
static int gettextprop(Window w, Atom atom, char *text, unsigned int size){
	char **list = NULL;
	int n;
	XTextProperty name;


	if(!text || size == 0)
		return 0;

	text[0] = '\0';

	if(!XGetTextProperty(dwm.dpy, w, &name, atom) || !name.nitems)
		return 0;

	if(name.encoding == XA_STRING){
		strncpy(text, (char*)name.value, size - 1);
	}
	else if(XmbTextPropertyToTextList(dwm.dpy, &name, &list, &n) >= Success && n > 0 && *list){
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);

	return 1;
}
