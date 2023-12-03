#ifndef ATOMS_H
#define ATOMS_H


#include <X11/X.h>
#include <X11/Xatom.h>
#include <xlib/window.h>


/* types */
typedef enum{
	NET_WM_NAME,
	NET_WM_CHECK,
	NET_WM_STATE,
	NET_WM_FULLSCREEN,
	NET_WM_VERTMAX,
	NET_WM_HORMAX,
	NET_WM_DESKTOP,
	NET_ACTIVE_WINDOW,
	NET_CLIENT_LIST,
	NET_SUPPORTED,
	NET_CURRENT_DESKTOP,
	NNETATOMS
} net_atom_t;

typedef enum{
	WM_DELETE_WINDOW,
	WM_PROTOCOLS,
	WM_STATE,
	WM_TAKEFOCUS,
	NWMATOMS
} wm_atom_t;

typedef struct{
	Atom property,
		 type;

	int format;
} atom_t;


/* prototypes */
void wmatom_init(wm_atom_t id, char const *name);
Atom wmatom_get(wm_atom_t id);

void netatom_init(net_atom_t id, char const *name, Atom type, int format);
Atom netatom_get(net_atom_t id);
void netatom_set(net_atom_t id, window_t win, unsigned char *v, int n);
void netatom_append(net_atom_t id, window_t win, unsigned char *v);
void netatom_delete(net_atom_t id, window_t win);


#endif // ATOMS_H
