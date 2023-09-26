#include <X11/Xft/Xft.h>
#include <action.h>
#include <client.h>
#include <config.h>
#include <config/config.h>
#include <dwm.h>
#include <events.h>
#include <layout.h>
#include <limits.h>
#include <monitor.h>
#include <signal.h>
#include <statusbar.h>
#include <stdlib.h>
#include <unistd.h>
#include <utils.h>


/* macros */
#define MOUSEMASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask)


/* global functions */
void action_focusstack(action_arg_t const *arg){
	client_t *c = NULL, *i;


	if(!dwm.selmon->sel || (dwm.selmon->sel->isfullscreen && CONFIG_LAYOUT_LOCK_FULLSCREEN))
		return;

	if(arg->i > 0){
		for(c=dwm.selmon->sel->next; c && !ISVISIBLE(c); c=c->next);

		if(!c){
			for(c=dwm.selmon->clients; c && !ISVISIBLE(c); c=c->next);
		}
	}
	else{
		for(i=dwm.selmon->clients; i!=dwm.selmon->sel; i=i->next){
			if(ISVISIBLE(i))
				c = i;
		}

		if(!c){
			for(; i; i=i->next){
				if(ISVISIBLE(i))
					c = i;
			}
		}
	}

	if(c){
		focus(c);
		restack(dwm.selmon);
	}
}

void action_killclient(action_arg_t const *arg){
	if(!dwm.selmon->sel)
		return;

	if(!sendevent(dwm.selmon->sel, dwm.wmatom[WMDelete]))
		killclient(dwm.selmon->sel->win);
}

void action_movemouse(action_arg_t const *arg){
	int x, y, ocx, ocy, nx, ny;
	client_t *c;
	monitor_t *m;
	XEvent ev;
	Time lasttime = 0;


	if(!(c = dwm.selmon->sel))
		return;

	if(c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(dwm.selmon);
	ocx = c->x;
	ocy = c->y;

	if(XGrabPointer(dwm.dpy, dwm.root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, dwm.cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	if(!getrootptr(&x, &y))
		return;

	do{
		XMaskEvent(dwm.dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch(ev.type){
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handle_event(&ev);
			break;

		case MotionNotify:
			if((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;

			lasttime = ev.xmotion.time;

			nx = ocx + (ev.xmotion.x - x);
			ny = ocy + (ev.xmotion.y - y);
			if(abs(dwm.selmon->wx - nx) < CONFIG_SNAP_PIXEL)
				nx = dwm.selmon->wx;
			else if(abs((dwm.selmon->wx + dwm.selmon->ww) - (nx + WIDTH(c))) < CONFIG_SNAP_PIXEL)
				nx = dwm.selmon->wx + dwm.selmon->ww - WIDTH(c);

			if(abs(dwm.selmon->wy - ny) < CONFIG_SNAP_PIXEL)
				ny = dwm.selmon->wy;
			else if(abs((dwm.selmon->wy + dwm.selmon->wh) - (ny + HEIGHT(c))) < CONFIG_SNAP_PIXEL)
				ny = dwm.selmon->wy + dwm.selmon->wh - HEIGHT(c);

			if(!dwm.selmon->lt[dwm.selmon->sellt]->arrange || c->isfloating)
				resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while(ev.type != ButtonRelease);
	
	XUngrabPointer(dwm.dpy, CurrentTime);
	
	if((m = recttomon(c->x, c->y, c->w, c->h)) != dwm.selmon){
		sendmon(c, m);
		dwm.selmon = m;
		focus(NULL);
	}
}

void action_moveclient(action_arg_t const *arg){
	int nx, ny;
	client_t *c;


	if(!(c = dwm.selmon->sel))
		return;

	if(c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(dwm.selmon);

	nx = ((int*)(arg->v))[0];
	ny = ((int*)(arg->v))[1];

	switch(nx){
	case -INT_MAX:	nx = dwm.selmon->mx; break;
	case INT_MAX:	nx = dwm.selmon->mx + dwm.selmon->mw - c->w - c->bw*2; break;
	default:		nx += c->x; break;
	}

	switch(ny){
	case -INT_MAX:	ny = dwm.selmon->my; break;
	case INT_MAX:	ny = dwm.selmon->my + dwm.selmon->mh - c->h - c->bw*2; break;
	default:		ny += c->y;
	}

	// TODO why do the following two lines not lead to the following glitch
	// 	- tiling mode with two windows
	// 	- resize the right windw
	// 	- move it to top-right
	// 	- move it to top-left
	// 	- move it bottom-left
	// 		=> focus moves to the other window
	resizeclient(c, nx, ny, c->w, c->h);
	focus(c);
//	if (!dwm.selmon->lt[dwm.selmon->sellt]->arrange || c->isfloating)
//		resize(c, nx, ny, c->w, c->h, 1);
//
//	if ((m = recttomon(c->x, c->y, c->w, c->h)) != dwm.selmon) {
//		sendmon(c, m);
//		dwm.selmon = m;
//		focus(NULL);
//	}
}

void action_reszclient(action_arg_t const *arg){
	int nx, ny, nw, nh;
	client_t *c;


	if(!(c = dwm.selmon->sel))
		return;

	if(c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(dwm.selmon);

	nw = ((int*)(arg->v))[0];
	nh = ((int*)(arg->v))[1];

	if(nw == INT_MAX){
		nx = dwm.selmon->mx;
		nw = dwm.selmon->mw - c->bw * 2;

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
		ny = dwm.selmon->my;
		nh = dwm.selmon->mh - c->bw * 2;

		if(c->y == ny && c->h == nh){
			ny = c->oldy;
			nh = c->oldh;
		}
	}
	else{
		ny = c->y - nh / 2;
		nh += c->h;
	}

	nw = MAX(nw, 32);
	nh = MAX(nh, 32);

	if(nw == c->w)
		nx = c->x;

	if(nh == c->h)
		ny = c->y;

	resizeclient(c, nx, ny, nw, nh);
	focus(c);
}

void action_quit(action_arg_t const *arg){
	dwm.running = 0;
}

void action_restart(action_arg_t const *arg){
	dwm.running = -1;
}

void action_resizemouse(action_arg_t const *arg){
	int ocx, ocy, nw, nh;
	client_t *c;
	monitor_t *m;
	XEvent ev;
	Time lasttime = 0;


	if(!(c = dwm.selmon->sel))
		return;

	if(c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;

	restack(dwm.selmon);
	ocx = c->x;
	ocy = c->y;

	if(XGrabPointer(dwm.dpy, dwm.root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, dwm.cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;

	XWarpPointer(dwm.dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);

	do{
		XMaskEvent(dwm.dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
		switch(ev.type){
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handle_event(&ev);
			break;

		case MotionNotify:
			if((ev.xmotion.time - lasttime) <= (1000 / 60))
				continue;

			lasttime = ev.xmotion.time;

			nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
			nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);

			if(!dwm.selmon->lt[dwm.selmon->sellt]->arrange || c->isfloating)
				resize(c, c->x, c->y, nw, nh, 1);

			break;
		}
	} while(ev.type != ButtonRelease);

	XWarpPointer(dwm.dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dwm.dpy, CurrentTime);

	while(XCheckMaskEvent(dwm.dpy, EnterWindowMask, &ev));

	if((m = recttomon(c->x, c->y, c->w, c->h)) != dwm.selmon){
		sendmon(c, m);
		dwm.selmon = m;
		focus(NULL);
	}
}

void action_setlayout(action_arg_t const *arg){
	if(!arg || !arg->v || arg->v != dwm.selmon->lt[dwm.selmon->sellt])
		dwm.selmon->sellt ^= 1;

	if(arg && arg->v)
		dwm.selmon->lt[dwm.selmon->sellt] = (layout_t*)arg->v;

	strncpy(dwm.selmon->ltsymbol, dwm.selmon->lt[dwm.selmon->sellt]->symbol, sizeof dwm.selmon->ltsymbol);

	if(dwm.selmon->sel)	arrange(dwm.selmon);
	else			drawbar(dwm.selmon);
}

void action_spawn(action_arg_t const *arg){
	struct sigaction sa;


	// TODO
	//	why is this needed
//	if(arg->v == dmenucmd)
//		dmenumon[0] = '0' + dwm.selmon->num;

	if(fork() == 0){
		if(dwm.dpy)
			close(ConnectionNumber(dwm.dpy));

		setsid();

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;
		sa.sa_handler = SIG_DFL;
		sigaction(SIGCHLD, &sa, NULL);

		execvp(((char **)arg->v)[0], (char **)arg->v);
		die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
	}
}

void action_tag(action_arg_t const *arg){
	if(dwm.selmon->sel && arg->ui & TAGMASK){
		dwm.selmon->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(dwm.selmon);
	}
}

void action_togglebar(action_arg_t const *arg){
	dwm.selmon->showbar = !dwm.selmon->showbar;
	updatebarpos(dwm.selmon);
	XMoveResizeWindow(dwm.dpy, dwm.selmon->barwin, dwm.selmon->wx, dwm.selmon->by, dwm.selmon->ww, dwm.statusbar_height);
	XMapRaised(dwm.dpy, dwm.selmon->barwin);
}

void action_togglefloating(action_arg_t const *arg){
	if(!dwm.selmon->sel)
		return;

	if(dwm.selmon->sel->isfullscreen) /* no support for fullscreen windows */
		return;

	dwm.selmon->sel->isfloating = !dwm.selmon->sel->isfloating || dwm.selmon->sel->isfixed;

	if(dwm.selmon->sel->isfloating)
		resize(dwm.selmon->sel, dwm.selmon->sel->x, dwm.selmon->sel->y, dwm.selmon->sel->w, dwm.selmon->sel->h, 0);

	arrange(dwm.selmon);
}

void action_toggletag(action_arg_t const *arg){
	unsigned int newtags;


	if(!dwm.selmon->sel)
		return;

	newtags = dwm.selmon->sel->tags ^ (arg->ui & TAGMASK);

	if(newtags){
		dwm.selmon->sel->tags = newtags;
		focus(NULL);
		arrange(dwm.selmon);
	}
}

void action_toggleview(action_arg_t const *arg){
	unsigned int newtagset = dwm.selmon->tagset[dwm.selmon->seltags] ^ (arg->ui & TAGMASK);


	if(newtagset){
		dwm.selmon->tagset[dwm.selmon->seltags] = newtagset;
		focus(NULL);
		arrange(dwm.selmon);
	}
}

void action_view(action_arg_t const *arg){
	if((arg->ui & TAGMASK) == dwm.selmon->tagset[dwm.selmon->seltags])
		return;

	dwm.selmon->seltags ^= 1; /* toggle sel tagset */

	if(arg->ui & TAGMASK)
		dwm.selmon->tagset[dwm.selmon->seltags] = arg->ui & TAGMASK;

	focus(NULL);
	arrange(dwm.selmon);
}
