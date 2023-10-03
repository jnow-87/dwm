#ifndef DWM_H
#define DWM_H


#include <X11/X.h>
#include <X11/Xlib.h>
#include <action.h>
#include <drw.h>


/* macros */
#define BUTTONMASK              (ButtonPressMask|ButtonReleaseMask)
#define CLEANMASK(mask)         (mask & ~(numlockmask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask))
#define INTERSECT(x,y,w,h,m)    (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
                               * MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))
#define ISVISIBLE(C)            ((C->tags & C->mon->tagset[C->mon->seltags]))
#define LENGTH(X)               (sizeof X / sizeof X[0])
#define MOUSEMASK               (BUTTONMASK|PointerMotionMask)
#define WIDTH(X)                ((X)->w + 2 * (X)->bw)
#define HEIGHT(X)               ((X)->h + 2 * (X)->bw)
#define TAGMASK                 ((1 << ntags) - 1)
#define TEXTW(X)                (drw_fontset_getwidth(drw, (X)) + lrpad)
#define MAX(A, B)               ((A) > (B) ? (A) : (B))
#define MIN(A, B)               ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))


/* incomplete types */
struct monitor_t;


/* types */
typedef enum{
	CurNormal,
	CurResize,
	CurMove,
	CurLast
} cursor_type_t;

typedef enum{
	SchemeNorm,
	SchemeSel
} color_scheme_t;

typedef enum{
	NetSupported,
	NetWMName,
	NetWMState,
	NetWMCheck,
	NetWMFullscreen,
	NetActiveWindow,
	NetWMWindowType,
	NetWMWindowTypeDialog,
	NetClientList,
	NetLast
} ewmh_atoms_t;

typedef enum{
	WMProtocols,
	WMDelete,
	WMState,
	WMTakeFocus,
	WMLast
} default_atoms_t;

typedef enum{
	ClkTagBar,
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkLast
} clicks_t;

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
	struct monitor_t *mon;
	Window win;
} client_t;

typedef struct{
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(const action_arg_t *arg);
	const action_arg_t arg;
} button_map_t;

typedef struct{
	unsigned int mod;
	KeySym keysym;
	void (*func)(const action_arg_t *);
	const action_arg_t arg;
} key_map_t;

typedef struct{
	const char *symbol;
	void (*arrange)(struct monitor_t *);
} layout_t;

typedef struct monitor_t{
	char ltsymbol[16];
	float mfact;
	int nmaster;
	int num;
	int by;               /* bar geometry */
	int mx, my, mw, mh;   /* screen size */
	int wx, wy, ww, wh;   /* window area  */
	unsigned int seltags;
	unsigned int sellt;
	unsigned int tagset[2];
	int showbar;
	int topbar;
	client_t *clients;
	client_t *sel;
	client_t *stack;
	struct monitor_t *next;
	Window barwin;
	const layout_t *lt[2];
} monitor_t;

typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} rule_t;


/* prototypes */
void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);void applyrules(client_t *c);
int applysizehints(client_t *c, int *x, int *y, int *w, int *h, int interact);
void arrange(monitor_t *m);
void arrangemon(monitor_t *m);
void attach(client_t *c);
void attachstack(client_t *c);
void buttonpress(XEvent *e);
void cleanup(void);
void cleanupmon(monitor_t *mon);
void clientmessage(XEvent *e);
void configure(client_t *c);
void configurenotify(XEvent *e);
void configurerequest(XEvent *e);
monitor_t *createmon(void);
void destroynotify(XEvent *e);
void detach(client_t *c);
void detachstack(client_t *c);
monitor_t *dirtomon(int dir);
void drawbar(monitor_t *m);
void drawbars(void);
void expose(XEvent *e);
void focus(client_t *c);
void focusin(XEvent *e);
Atom getatomprop(client_t *c, Atom prop);
int getrootptr(int *x, int *y);
long getstate(Window w);
int gettextprop(Window w, Atom atom, char *text, unsigned int size);
void grabbuttons(client_t *c, int focused);
void grabkeys(void);
void keypress(XEvent *e);
void manage(Window w, XWindowAttributes *wa);
void mappingnotify(XEvent *e);
void maprequest(XEvent *e);
void monocle(monitor_t *m);
void motionnotify(XEvent *e);
client_t *nexttiled(client_t *c);
void pop(client_t *c);
void propertynotify(XEvent *e);
monitor_t *recttomon(int x, int y, int w, int h);
void resize(client_t *c, int x, int y, int w, int h, int interact);
void resizeclient(client_t *c, int x, int y, int w, int h);
void restack(monitor_t *m);
void run(void);
void scan(void);
int sendevent(client_t *c, Atom proto);
void sendmon(client_t *c, monitor_t *m);
void setclientstate(client_t *c, long state);
void setfocus(client_t *c);
void setfullscreen(client_t *c, int fullscreen);
void setup(void);
void seturgent(client_t *c, int urg);
void showhide(client_t *c);
void tile(monitor_t *m);
void unfocus(client_t *c, int setfocus);
void unmanage(client_t *c, int destroyed);
void unmapnotify(XEvent *e);
void updatebarpos(monitor_t *m);
void updatebars(void);
void updateclientlist(void);
int updategeom(void);
void updatenumlockmask(void);
void updatesizehints(client_t *c);
void updatestatus(void);
void updatetitle(client_t *c);
void updatewindowtype(client_t *c);
void updatewmhints(client_t *c);
client_t *wintoclient(Window w);
monitor_t *wintomon(Window w);
void killclient(Window win);


/* external variables */
extern monitor_t *mons,
				 *selmon;
extern const int lockfullscreen;
extern Atom wmatom[];
extern Display *dpy;
extern Window root;
extern cursor_t *cursor[];
extern void (*handler[]) (XEvent *);
extern int running;
extern const char *tags[];
extern unsigned int ntags;	// TODO replace with LENGTH(tags)
extern const unsigned int snap;
extern const char *dmenucmd[];
extern char dmenumon[];
extern int bar_height;


#endif // DWM_H
