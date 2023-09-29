#include <X11/extensions/Xinerama.h>
#include <sys/types.h>
#include <client.h>
#include <monitor.h>
#include <statusbar.h>
#include <dwm.h>
#include <utils.h>
#include <log.h>


/* local/static prototypes */
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info);


/* global functions */
int xinerama_discover_monitor(void){
	bool dirty = false;
	int i,
		nmon,
		ninfo,
		nunique;
	client_t *c;
	monitor_t *m;
	XineramaScreenInfo *info;
	XineramaScreenInfo *unique = NULL;


	info = XineramaQueryScreens(dwm.dpy, &ninfo);
	DEBUG("number of xinerama monitors: %d\n", ninfo);

	if(info == 0x0)
		return -1;

	/* only consider unique geometries as separate screens */
	unique = ecalloc(ninfo, sizeof(XineramaScreenInfo));

	for(i=0, nunique=0; i<ninfo; i++){
		if(isuniquegeom(unique, nunique, &info[i]))
			memcpy(&unique[nunique++], &info[i], sizeof(XineramaScreenInfo));
	}

	XFree(info);
	ninfo = nunique;
	DEBUG("number of unique geometries: %d\n", nunique);

	/* new monitors if ninfo > nmon */
	for(nmon=0, m=dwm.mons; m; m=m->next, nmon++);
	DEBUG("number of previous monitor: %d\n", nmon);

	for(i=nmon; i<ninfo; i++){
		for(m=dwm.mons; m && m->next; m=m->next);

		if(m)	m->next = createmon();
		else	dwm.mons = createmon();
	}

	for(i=0, m=dwm.mons; i<ninfo && m; m=m->next, i++){
		if(i >= nmon || unique[i].x_org != m->x || unique[i].y_org != m->y || unique[i].width != m->width || unique[i].height != m->height){
			dirty = true;

			m->x = unique[i].x_org;
			m->y = unique[i].y_org;
			m->width = unique[i].width;
			m->height = unique[i].height;

			DEBUG("init monitor %d:\n", i);
			DEBUG("  screen area: %dx%d+%d+%d\n", m->x, m->y, m->width, m->height);
			DEBUG("  window area: %dx%d+%d+%d\n", m->x, m->y, m->width, m->height);
		}
	}

	/* removed monitors if nmon > ninfo */
	for(i=ninfo; i<nmon; i++){
		for(m=dwm.mons; m && m->next; m=m->next);

		DEBUG("remove monitor\n");

		while((c = dwm.clients)){
			dirty = true;
			dwm.clients = c->next;
			detachstack(c);
			attach(c);
			attachstack(c);
		}

		cleanupmon(m);
	}

	free(unique);

	return dirty;
}


/* local functions */
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info){
	while(n--){
		if(unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	}

	return 1;
}
