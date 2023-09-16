#ifndef CLIENT_H
#define CLIENT_H


#include <monitor.h>


/* macros */
#define ISVISIBLE(C)	((C->tags & C->mon->tagset[C->mon->seltags]))
#define WIDTH(X)		((X)->w + 2 * (X)->bw)
#define HEIGHT(X)		((X)->h + 2 * (X)->bw)



/* types */
typedef struct client_t{
	char name[256];
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
	int isfixed, isfloating, isurgent, neverfocus, oldstate, isfullscreen;
	struct client_t *next;
	struct client_t *snext;
	monitor_t *mon;
	Window win;
} client_t;


/* prototypes */
void applyrules(client_t *c);
int applysizehints(client_t *c, int *x, int *y, int *w, int *h, int interact);
void attach(client_t *c);
void attachstack(client_t *c);
void configure(client_t *c);
void detach(client_t *c);
void detachstack(client_t *c);
void focus(client_t *c);
Atom getatomprop(client_t *c, Atom prop);
void grabbuttons(client_t *c, int focused);
void pop(client_t *c);
void resize(client_t *c, int x, int y, int w, int h, int interact);
void resizeclient(client_t *c, int x, int y, int w, int h);
void sendmon(client_t *c, monitor_t *m);
void setclientstate(client_t *c, long state);
int sendevent(client_t *c, Atom proto);
void setfocus(client_t *c);
void setfullscreen(client_t *c, int fullscreen);
void seturgent(client_t *c, int urg);
void showhide(client_t *c);
void unfocus(client_t *c, int setfocus);
void unmanage(client_t *c, int destroyed);
void updatesizehints(client_t *c);
void updatewindowtype(client_t *c);
void updatewmhints(client_t *c);
void killclient(Window win);
client_t *wintoclient(Window w);


#endif // CLIENT_H
