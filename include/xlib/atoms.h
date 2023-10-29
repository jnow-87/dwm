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
	NET_ACTIVE_WINDOW,
	NET_CLIENT_LIST,
	NET_SUPPORTED,
	NNETATOMS
} net_atom_t;

typedef enum{
	WM_PROTOCOLS,
	WM_DELETE,
	WM_STATE,
	WM_TAKEFOCUS,
	NWMATOMS
} wm_atoms_t;


/* prototypes */
int atoms_text_prop(window_t win, Atom atom, char *text, unsigned int size);
int atoms_text_prop_set(window_t win, Atom atom, char *text);

void atoms_netatom_append(net_atom_t atom, unsigned char *value);
void atoms_netatom_delete(net_atom_t atom);


#endif // ATOMS_H
