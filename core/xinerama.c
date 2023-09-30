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
	int i,
		ninfo,
		nunique;
	monitor_t *m;
	XineramaScreenInfo *info,
					   *unique;


	info = XineramaQueryScreens(dwm.dpy, &ninfo);
	DEBUG("number of xinerama monitors: %d\n", ninfo);

	if(info == 0x0)
		return -1;

	/* only consider unique geometries as separate screens */
	nunique = 0;
	unique = malloc(ninfo * sizeof(XineramaScreenInfo));

	if(unique == 0x0)
		die("out of memory\n");

	for(i=0; i<ninfo; i++){
		if(isuniquegeom(unique, nunique, &info[i]))
			memcpy(&unique[nunique++], &info[i], sizeof(XineramaScreenInfo));
	}

	XFree(info);
	ninfo = nunique;
	DEBUG("number of unique geometries: %d\n", nunique);

	/* allccate monitors */
	for(i=0; i<ninfo; i++){
		info = unique + ninfo - i - 1;
		m = monitor_create(info->x_org, info->y_org, info->width, info->height);

		if(m == 0x0)
			die("unable to create monitor\n");

		DEBUG("init monitor %d:\n", i);
		DEBUG("  screen area: %dx%d+%d+%d\n", m->x, m->y, m->width, m->height);
	}

	free(unique);

	return 1;
}


/* local functions */
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info){
	while(n--){
		if(unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width && unique[n].height == info->height)
			return 0;
	}

	return 1;
}
