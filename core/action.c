#include <config/config.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <X11/Xft/Xft.h>
#include <dwm.h>
#include <action.h>
#include <layout.h>
#include <statusbar.h>


/* global functions */
void action_focusmon(const action_arg_t *arg){
	monitor_t *m;

	if (!mons->next)
		return;
	if ((m = dirtomon(arg->i)) == selmon)
		return;
	unfocus(selmon->sel, 0);
	selmon = m;
	focus(NULL);
}

void action_focusstack(const action_arg_t *arg){
	client_t *c = NULL, *i;

	if (!selmon->sel || (selmon->sel->isfullscreen && CONFIG_LAYOUT_LOCK_FULLSCREEN))
		return;
	if (arg->i > 0) {
		for (c = selmon->sel->next; c && !ISVISIBLE(c); c = c->next);
		if (!c)
			for (c = selmon->clients; c && !ISVISIBLE(c); c = c->next);
	} else {
		for (i = selmon->clients; i != selmon->sel; i = i->next)
			if (ISVISIBLE(i))
				c = i;
		if (!c)
			for (; i; i = i->next)
				if (ISVISIBLE(i))
					c = i;
	}
	if (c) {
		focus(c);
		restack(selmon);
	}
}

void action_incnmaster(const action_arg_t *arg){
	selmon->nmaster = MAX(selmon->nmaster + arg->i, 0);
	arrange(selmon);
}

void action_killclient(const action_arg_t *arg){
	if (!selmon->sel)
		return;
	if (!sendevent(selmon->sel, wmatom[WMDelete])) {
		killclient(selmon->sel->win);
	}
}

void action_movemouse(const action_arg_t *arg){
	int x, y, ocx, ocy, nx, ny;
	client_t *c;
	monitor_t *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;
	if (!getrootptr(&x, &y))
		return;
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if (abs(selmon->wx - nx) < CONFIG_SNAP_PIXEL)
				nx = selmon->wx;
			else if (abs((selmon->wx + selmon->ww) - (nx + WIDTH(c))) < CONFIG_SNAP_PIXEL)
				nx = selmon->wx + selmon->ww - WIDTH(c);
			if (abs(selmon->wy - ny) < CONFIG_SNAP_PIXEL)
				ny = selmon->wy;
			else if (abs((selmon->wy + selmon->wh) - (ny + HEIGHT(c))) < CONFIG_SNAP_PIXEL)
				ny = selmon->wy + selmon->wh - HEIGHT(c);
			if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
			&& (abs(nx - c->x) > CONFIG_SNAP_PIXEL || abs(ny - c->y) > CONFIG_SNAP_PIXEL))
				action_togglefloating(NULL);
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XUngrabPointer(dpy, CurrentTime);
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void action_moveclient(const action_arg_t *arg){
	int nx, ny;
	client_t *c;

	if (!(c = selmon->sel))
		return;

	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(selmon);

	nx = ((int*)(arg->v))[0];
	ny = ((int*)(arg->v))[1];

	switch(nx){
	case -INT_MAX:	nx = selmon->mx; break;
	case INT_MAX:	nx = selmon->mx + selmon->mw - c->w - c->bw*2; break;
	default:		nx += c->x; break;
	}

	switch(ny){
	case -INT_MAX:	ny = selmon->my; break;
	case INT_MAX:	ny = selmon->my + selmon->mh - c->h - c->bw*2; break;
	default:		ny += c->y;
	}

	if (!c->isfloating)
		action_togglefloating(NULL);

	// TODO why do the following two lines not lead to the following glitch
	// 	- tiling mode with two windows
	// 	- resize the right windw
	// 	- move it to top-right
	// 	- move it to top-left
	// 	- move it bottom-left
	// 		=> focus moves to the other window
	resizeclient(c, nx, ny, c->w, c->h);
	focus(c);
//	if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
//		resize(c, nx, ny, c->w, c->h, 1);
//
//	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
//		sendmon(c, m);
//		selmon = m;
//		focus(NULL);
//	}
}

void action_reszclient(const action_arg_t *arg){
	int nx, ny, nw, nh;
	client_t *c;

	if (!(c = selmon->sel))
		return;

	if (c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(selmon);

	nw = ((int*)(arg->v))[0];
	nh = ((int*)(arg->v))[1];

	if(nw == INT_MAX){
		nx = selmon->mx;
		nw = selmon->mw - c->bw * 2;

		if(c->x == nx && c->w == nw){
			nx = c->oldx;
			nw = c->oldw;
		}

	}
	else{
		nx = c->x - nw / 2;
		nw += c->w;
	}

	if(nh == INT_MAX){
		ny = selmon->my;
		nh = selmon->mh - c->bw * 2;

		if(c->y == ny && c->h == nh){
			ny = c->oldy;
			nh = c->oldh;
		}

	}
	else{
		ny = c->y - nh / 2;
		nh += c->h;
	}

	if (!c->isfloating)
		action_togglefloating(NULL);

	nw = MAX(nw, 32);
	nh = MAX(nh, 32);

	if(nw == c->w)
		nx = c->x;

	if(nh == c->h)
		ny = c->y;

	resizeclient(c, nx, ny, nw, nh);
	focus(c);
}

void action_quit(const action_arg_t *arg){
	running = 0;
}

void action_restart(const action_arg_t *arg){
	running = -1;
}

void action_resizemouse(const action_arg_t *arg){
	int ocx, ocy, nw, nh;
	client_t *c;
	monitor_t *m;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;
	if (c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;
	restack(selmon);
	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch(ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;
			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
			if (c->mon->wx + nw >= selmon->wx && c->mon->wx + nw <= selmon->wx + selmon->ww
			&& c->mon->wy + nh >= selmon->wy && c->mon->wy + nh <= selmon->wy + selmon->wh)
			{
				if (!c->isfloating && selmon->lt[selmon->sellt]->arrange
				&& (abs(nw - c->w) > CONFIG_SNAP_PIXEL || abs(nh - c->h) > CONFIG_SNAP_PIXEL))
					action_togglefloating(NULL);
			}
			if (!selmon->lt[selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);
			break;
		}
	} while (ev.type != ButtonRelease);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
	if ((m = recttomon(c->x, c->y, c->w, c->h)) != selmon) {
		sendmon(c, m);
		selmon = m;
		focus(NULL);
	}
}

void action_setlayout(const action_arg_t *arg){
	if (!arg || !arg->v || arg->v != selmon->lt[selmon->sellt])
		selmon->sellt ^= 1;
	if (arg && arg->v)
		selmon->lt[selmon->sellt] = (layout_t *)arg->v;
	strncpy(selmon->ltsymbol, selmon->lt[selmon->sellt]->symbol, sizeof selmon->ltsymbol);
	if (selmon->sel)
		arrange(selmon);
	else
		drawbar(selmon);
}

void action_setmfact(const action_arg_t *arg){
	float f;

	if (!arg || !selmon->lt[selmon->sellt]->arrange)
		return;

	/* arg > 1.0 will set mfact absolutely */
	f = arg->f < 1.0 ? arg->f + selmon->mfact : arg->f - 1.0;
	if (f < 0.05 || f > 0.95)
		return;
	selmon->mfact = f;
	arrange(selmon);
}

void action_spawn(const action_arg_t *arg){
	struct sigaction sa;

	if (arg->v == dmenucmd)
		dmenumon[0] = '0' + selmon->num;
	if (fork() == 0) {
		if (dpy)
			close(ConnectionNumber(dpy));
		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

void action_tag(const action_arg_t *arg){
	if (selmon->sel && arg->ui & TAGMASK) {
		selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(selmon);
	}
}

void action_tagmon(const action_arg_t *arg){
	if (!selmon->sel || !mons->next)
		return;
	sendmon(selmon->sel, dirtomon(arg->i));
}

void action_togglebar(const action_arg_t *arg){
	selmon->showbar = !selmon->showbar;
	updatebarpos(selmon);
	XMoveResizeWindow(dpy, selmon->barwin, selmon->wx, selmon->by, selmon->ww, bar_height);
	XMapRaised(dpy, selmon->barwin);
}

void action_togglefloating(const action_arg_t *arg){
	if (!selmon->sel)
		return;
	if (selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;
	selmon->sel->isfloating = !selmon->sel->isfloating || selmon->sel->isfixed;
	if (selmon->sel->isfloating)
		resize(selmon->sel, selmon->sel->x, selmon->sel->y,
			selmon->sel->w, selmon->sel->h, 0);
	arrange(selmon);
}

void action_toggletag(const action_arg_t *arg){
	unsigned int newtags;

	if (!selmon->sel)
		return;
	newtags = selmon->sel->tags ^ (arg->ui & TAGMASK);
	if (newtags) {
		selmon->sel->tags = newtags;
		focus(NULL);
		arrange(selmon);
	}
}

void action_toggleview(const action_arg_t *arg){
	unsigned int newtagset = selmon->tagset[selmon->seltags] ^ (arg->ui & TAGMASK);

	if (newtagset) {
		selmon->tagset[selmon->seltags] = newtagset;
		focus(NULL);
		arrange(selmon);
	}
}

void action_view(const action_arg_t *arg){
	if ((arg->ui & TAGMASK) == selmon->tagset[selmon->seltags])
		return;
	selmon->seltags ^= 1; /* toggle sel tagset */
	if (arg->ui & TAGMASK)
		selmon->tagset[selmon->seltags] = arg->ui & TAGMASK;
	focus(NULL);
	arrange(selmon);
}

void action_zoom(const action_arg_t *arg){
	client_t *c = selmon->sel;

	if (!selmon->lt[selmon->sellt]->arrange || !c || c->isfloating)
		return;
	if (c == nexttiled(selmon->clients) && !(c = nexttiled(c->next)))
		return;
	pop(c);
}
