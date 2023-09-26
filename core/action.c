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


	if(!dwm.mons->sel || (dwm.mons->sel->isfullscreen && CONFIG_LAYOUT_LOCK_FULLSCREEN))
		return;

	if(arg->i > 0){
		for(c=dwm.mons->sel->next; c && !ISVISIBLE(c); c=c->next);

		if(!c){
			for(c=dwm.mons->clients; c && !ISVISIBLE(c); c=c->next);
		}
	}
	else{
		for(i=dwm.mons->clients; i!=dwm.mons->sel; i=i->next){
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
		restack(dwm.mons);
	}

	statusbar_draw();
}

void action_killclient(action_arg_t const *arg){
	if(!dwm.mons->sel)
		return;

	if(!sendevent(dwm.mons->sel, dwm.wmatom[WMDelete]))
		killclient(dwm.mons->sel->win);
}

void action_movemouse(action_arg_t const *arg){
	int x, y, ocx, ocy, nx, ny;
	client_t *c;
	XEvent ev;
	Time lasttime = 0;


	if(!(c = dwm.mons->sel))
		return;

	if(c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(dwm.mons);
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
			if(abs(dwm.mons->wx - nx) < CONFIG_SNAP_PIXEL)
				nx = dwm.mons->wx;
			else if(abs((dwm.mons->wx + dwm.mons->ww) - (nx + WIDTH(c))) < CONFIG_SNAP_PIXEL)
				nx = dwm.mons->wx + dwm.mons->ww - WIDTH(c);

			if(abs(dwm.mons->wy - ny) < CONFIG_SNAP_PIXEL)
				ny = dwm.mons->wy;
			else if(abs((dwm.mons->wy + dwm.mons->wh) - (ny + HEIGHT(c))) < CONFIG_SNAP_PIXEL)
				ny = dwm.mons->wy + dwm.mons->wh - HEIGHT(c);

			resize(c, nx, ny, c->w, c->h, 1);
			break;
		}
	} while(ev.type != ButtonRelease);
	
	XUngrabPointer(dwm.dpy, CurrentTime);
}

void action_moveclient(action_arg_t const *arg){
	int nx, ny;
	client_t *c;


	if(!(c = dwm.mons->sel))
		return;

	if(c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(dwm.mons);

	nx = ((int*)(arg->v))[0];
	ny = ((int*)(arg->v))[1];

	switch(nx){
	case -INT_MAX:	nx = dwm.mons->mx; break;
	case INT_MAX:	nx = dwm.mons->mx + dwm.mons->mw - c->w - c->bw*2; break;
	default:		nx += c->x; break;
	}

	switch(ny){
	case -INT_MAX:	ny = dwm.mons->my; break;
	case INT_MAX:	ny = dwm.mons->my + dwm.mons->mh - c->h - c->bw*2; break;
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
}

void action_reszclient(action_arg_t const *arg){
	int nx, ny, nw, nh;
	client_t *c;


	if(!(c = dwm.mons->sel))
		return;

	if(c->isfullscreen) /* no support moving fullscreen windows by mouse */
		return;

	restack(dwm.mons);

	nw = ((int*)(arg->v))[0];
	nh = ((int*)(arg->v))[1];

	if(nw == INT_MAX){
		nx = dwm.mons->mx;
		nw = dwm.mons->mw - c->bw * 2;

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
		ny = dwm.mons->my;
		nh = dwm.mons->mh - c->bw * 2;

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
	XEvent ev;
	Time lasttime = 0;


	if(!(c = dwm.mons->sel))
		return;

	if(c->isfullscreen) /* no support resizing fullscreen windows by mouse */
		return;

	restack(dwm.mons);
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

			resize(c, c->x, c->y, nw, nh, 1);

			break;
		}
	} while(ev.type != ButtonRelease);

	XWarpPointer(dwm.dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
	XUngrabPointer(dwm.dpy, CurrentTime);

	while(XCheckMaskEvent(dwm.dpy, EnterWindowMask, &ev));
}

void action_setlayout(action_arg_t const *arg){
	if(!arg || !arg->v || arg->v != dwm.mons->lt[dwm.mons->sellt])
		dwm.mons->sellt ^= 1;

	if(arg && arg->v)
		dwm.mons->lt[dwm.mons->sellt] = (layout_t*)arg->v;

	strncpy(dwm.mons->ltsymbol, dwm.mons->lt[dwm.mons->sellt]->symbol, sizeof dwm.mons->ltsymbol);

	if(dwm.mons->sel)
		arrange(dwm.mons);

	statusbar_draw();
}

void action_spawn(action_arg_t const *arg){
	struct sigaction sa;


	// TODO
	//	why is this needed
//	if(arg->v == dmenucmd)
//		dmenumon[0] = '0' + dwm.mons->num;

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
	if(dwm.mons->sel && arg->ui & TAGMASK){
		dwm.mons->sel->tags = arg->ui & TAGMASK;
		focus(NULL);
		arrange(dwm.mons);
	}
}

void action_togglebar(action_arg_t const *arg){
	statusbar_toggle();
}

void action_toggletag(action_arg_t const *arg){
	unsigned int newtags;


	if(!dwm.mons->sel)
		return;

	newtags = dwm.mons->sel->tags ^ (arg->ui & TAGMASK);

	if(newtags){
		dwm.mons->sel->tags = newtags;
		focus(NULL);
		arrange(dwm.mons);
	}
}

void action_toggleview(action_arg_t const *arg){
	unsigned int newtagset = dwm.mons->tagset[dwm.mons->seltags] ^ (arg->ui & TAGMASK);


	if(newtagset){
		dwm.mons->tagset[dwm.mons->seltags] = newtagset;
		focus(NULL);
		arrange(dwm.mons);
	}
}

void action_view(action_arg_t const *arg){
	if((arg->ui & TAGMASK) == dwm.mons->tagset[dwm.mons->seltags])
		return;

	dwm.mons->seltags ^= 1; /* toggle sel tagset */

	if(arg->ui & TAGMASK)
		dwm.mons->tagset[dwm.mons->seltags] = arg->ui & TAGMASK;

	focus(NULL);
	arrange(dwm.mons);
	statusbar_draw();
}
