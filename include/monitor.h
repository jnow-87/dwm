#ifndef MONITOR_H
#define MONITOR_H


#include <X11/X.h>


/* incomplete types */
struct layout_t;
struct client_t;


/* types */
typedef struct monitor_t{
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;				/* bar geometry */
	int mx, my, mw, mh;	/* screen size */
	int wx, wy, ww, wh;	/* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	struct client_t *clients;
	struct client_t *sel;
	struct client_t *stack;
	struct monitor_t *next;
	Window barwin;
	const struct layout_t *lt[2];
} monitor_t;


/* prototypes */
monitor_t *createmon(void);
monitor_t *dirtomon(int dir);
monitor_t *recttomon(int x, int y, int w, int h);
monitor_t *wintomon(Window w);
void cleanupmon(monitor_t *mon);
void restack(monitor_t *m);


#endif // MONITOR_H
