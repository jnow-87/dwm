#ifndef ATOMS_H
#define ATOMS_H


#include <X11/X.h>
#include <X11/Xatom.h>
#include <xlib/window.h>


/* types */
typedef enum{
	NET_SUPPORTED,
	NET_WMNAME,
	NET_WMCHECK,
	NET_WMSTATE,
	NET_WMFULLSCREEN,
	NET_WMVMAXIMIZED,
	NET_WMHMAXIMIZED,
	NET_ACTIVEWINDOW,
	NET_CLIENTLIST,
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
int atoms_text_prop_set(Window win, Atom atom, char *text);

void atoms_netatom_append(net_atom_t atom, unsigned char *value);
void atoms_netatom_delete(net_atom_t atom);


#endif // ATOMS_H
