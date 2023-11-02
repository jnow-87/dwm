#include <string.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <core/dwm.h>


/* local/static prototypes */
void netatom_change(window_t win, net_atom_t id, int mode, unsigned char *v, int n);


/* global functions */
void wmatom_init(wm_atom_t id, char const *name){
	atom_t *atom = dwm.wmatoms + id;


	atom->property = XInternAtom(dwm.dpy, name, False);
	atom->type = None;
	atom->format = -1;
}

Atom wmatom_get(wm_atom_t atom){
	return dwm.wmatoms[atom].property;
}

void netatom_init(net_atom_t id, char const *name, Atom type, int format){
	atom_t *atom = dwm.netatoms + id;


	atom->property = XInternAtom(dwm.dpy, name, False);
	atom->type = type;
	atom->format = format;
}

Atom netatom_get(net_atom_t id){
	return dwm.netatoms[id].property;
}

void netatom_set(net_atom_t id, window_t win, unsigned char *v, int n){
	netatom_change(win, id, PropModeReplace, v, n);
}

void netatom_append(net_atom_t id, window_t win, unsigned char *v){
	netatom_change(win, id, PropModeAppend, v, 1);
}

void netatom_delete(net_atom_t id, window_t win){
	XDeleteProperty(dwm.dpy, win, dwm.netatoms[id].property);
}


/* local functions */
void netatom_change(window_t win, net_atom_t id, int mode, unsigned char *v, int n){
	atom_t *atom = dwm.netatoms + id;


	XChangeProperty(dwm.dpy, win, atom->property, atom->type, atom->format, mode, v, n);
}
