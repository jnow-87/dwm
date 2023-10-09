#ifndef ATOMS_H
#define ATOMS_H


#include <X11/X.h>
#include <X11/Xatom.h>
#include <xlib/window.h>


/* types */
typedef enum{
	NetSupported,
	NetWMName,
	NetWMCheck,
	NetActiveWindow,
	NetClientList,
	NetLast
} net_atom_t;

typedef enum{
	WMProtocols,
	WMDelete,
	WMState,
	WMTakeFocus,
	WMLast
} wm_atoms_t;


/* prototypes */
int atoms_text_prop(window_t win, Atom atom, char *text, unsigned int size);

void atoms_netatom_append(net_atom_t atom, unsigned char *value);
void atoms_netatom_delete(net_atom_t atom);


#endif // ATOMS_H
