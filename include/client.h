#ifndef CLIENT_H
#define CLIENT_H


#include <X11/Xlib.h>
#include <monitor.h>


/* macros */
#define ISVISIBLE(C)	((C->tags & C->mon->tagset[C->mon->seltags]))
#define WIDTH(X)		((X)->w + 2 * (X)->bw)
#define HEIGHT(X)		((X)->h + 2 * (X)->bw)


/* types */
typedef struct client_t{
	float mina, maxa;
	int x, y, w, h;
	int oldx, oldy, oldw, oldh;
	int basew, baseh, incw, inch, maxw, maxh, minw, minh, hintsvalid;
	int bw, oldbw;
	unsigned int tags;
	int isfixed, isurgent, neverfocus;
	monitor_t *mon;
	Window win;

	struct client_t *next;			// client list per monitor
	struct client_t *stack_next;	// client stack per monitor
} client_t;


/* prototypes */
client_t *wintoclient(Window w);

void manage(Window w, XWindowAttributes *wa);
void unmanage(client_t *c, int destroyed);
void killclient(Window win);

void configure(client_t *c);

void attach(client_t *c);
void attachstack(client_t *c);
// TODO detach
void detachstack(client_t *c);
void pop(client_t *c);

void focus(client_t *c);
void unfocus(client_t *c, int setfocus);
void showhide(client_t *c);
void sendmon(client_t *c, monitor_t *m);

void resize(client_t *c, int x, int y, int w, int h, int interact);
void resizeclient(client_t *c, int x, int y, int w, int h);
void setclientstate(client_t *c, long state);
void setfocus(client_t *c);
void seturgent(client_t *c, int urg);

int sendevent(client_t *c, Atom proto);

void updatewmhints(client_t *c);


#endif // CLIENT_H
