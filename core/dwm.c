/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * The event handlers of dwm are organized in an array which is accessed
 * whenever a new event has been fetched. This allows event dispatching
 * in O(1) time.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized in a linked client
 * list on each monitor, the focus history is remembered through a stack list
 * on each monitor. Each client contains a bit array to indicate the tags of a
 * client.
 *
 * Keys and tagging rules are organized as arrays and defined in config.h.
 *
 * To understand everything else, start reading main().
 */

#include <X11/X.h>
#include <X11/Xatom.h>
#include <config/config.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#ifdef CONFIG_XINERAMA
# include <X11/extensions/Xinerama.h>
#endif /* CONFIG_XINERAMA */
#include <client.h>
#include <colors.h>
#include <config.h>
#include <dwm.h>
#include <monitor.h>
#include <statusbar.h>
#include <utils.h>


/* local/static prototypes */
#ifdef CONFIG_XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info);
#endif // CONFIG_XINERAMA


/* global functions */
void die(char const *fmt, ...){
	va_list ap;


	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if(fmt[0] && fmt[strlen(fmt) - 1] == ':'){
		fputc(' ', stderr);
		perror(NULL);
	}
	else
		fputc('\n', stderr);

	exit(1);
}

int getrootptr(int *x, int *y){
	int di;
	unsigned int dui;
	Window dummy;


	return XQueryPointer(dwm.dpy, dwm.root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void grabkeys(void){
	updatenumlockmask();
	{
		unsigned int i, j, k;
		unsigned int modifiers[] = { 0, LockMask, dwm.numlock_mask, dwm.numlock_mask | LockMask };
		int start, end, skip;
		KeySym *syms;


		XUngrabKey(dwm.dpy, AnyKey, AnyModifier, dwm.root);
		XDisplayKeycodes(dwm.dpy, &start, &end);
		syms = XGetKeyboardMapping(dwm.dpy, start, end - start + 1, &skip);
		if(!syms)
			return;
		for(k=start; k<=end; k++)
			for(i=0; i<nkeys; i++)
				/* skip modifier codes, we do that ourselves */
				if(keys[i].keysym == syms[(k - start) * skip])
					for(j=0; j<LENGTH(modifiers); j++)
						XGrabKey(dwm.dpy, k, keys[i].mod | modifiers[j], dwm.root, True, GrabModeAsync, GrabModeAsync);
		XFree(syms);
	}
}

int updategeom(void){
	int dirty = 0;
	int i, j, n, nn;


#ifdef CONFIG_XINERAMA
	if(XineramaIsActive(dwm.dpy)){
		client_t *c;
		monitor_t *m;
		XineramaScreenInfo *info = XineramaQueryScreens(dwm.dpy, &nn);
		XineramaScreenInfo *unique = NULL;

		for(n=0, m=dwm.mons; m; m=m->next, n++);

		/* only consider unique geometries as separate screens */
		unique = ecalloc(nn, sizeof(XineramaScreenInfo));

		for(i=0, j=0; i<nn; i++){
			if(isuniquegeom(unique, j, &info[i]))
				memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
		}

		XFree(info);
		nn = j;

		/* new monitors if nn > n */
		for(i=n; i<nn; i++){
			for(m=dwm.mons; m && m->next; m=m->next);

			if(m)	m->next = createmon();
			else	dwm.mons = createmon();
		}

		for(i=0, m=dwm.mons; i<nn && m; m=m->next, i++){
			if(i >= n
				|| unique[i].x_org != m->mx || unique[i].y_org != m->my
				|| unique[i].width != m->mw || unique[i].height != m->mh){
				dirty = 1;
				m->num = i;
				m->mx = m->wx = unique[i].x_org;
				m->my = m->wy = unique[i].y_org;
				m->mw = m->ww = unique[i].width;
				m->mh = m->wh = unique[i].height;
				updatebarpos(m);
			}
		}

		/* removed monitors if n > nn */
		for(i=nn; i<n; i++){
			for(m=dwm.mons; m && m->next; m=m->next);

			while((c = m->clients)){
				dirty = 1;
				m->clients = c->next;
				detachstack(c);
				c->mon = dwm.mons;
				attach(c);
				attachstack(c);
			}

			if(m == dwm.selmon)
				dwm.selmon = dwm.mons;

			cleanupmon(m);
		}

		free(unique);
	}
	else
#endif /* CONFIG_XINERAMA */
	{  /* default monitor setup */
		if(!dwm.mons)
			dwm.mons = createmon();

		if(dwm.mons->mw != dwm.screen_width || dwm.mons->mh != dwm.screen_height){
			dirty = 1;
			dwm.mons->mw = dwm.mons->ww = dwm.screen_width;
			dwm.mons->mh = dwm.mons->wh = dwm.screen_height;
			updatebarpos(dwm.mons);
		}
	}

	if(dirty){
		dwm.selmon = dwm.mons;
		dwm.selmon = wintomon(dwm.root);
	}

	return dirty;
}

void updatenumlockmask(void){
	unsigned int i, j;
	XModifierKeymap *modmap;


	dwm.numlock_mask = 0;
	modmap = XGetModifierMapping(dwm.dpy);

	for(i=0; i<8; i++){
		for(j=0; j<modmap->max_keypermod; j++){
			if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dwm.dpy, XK_Num_Lock))
				dwm.numlock_mask = (1 << i);
		}
	}

	XFreeModifiermap(modmap);
}


/* local functions */
#ifdef CONFIG_XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info){
	while(n--){
		if(unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	}

	return 1;
}
#endif /* CONFIG_XINERAMA */
