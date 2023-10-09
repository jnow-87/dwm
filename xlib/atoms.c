#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <core/dwm.h>


/* global functions */
int atoms_text_prop(Window w, Atom atom, char *text, unsigned int size){
	char **list = 0x0;
	int n;
	XTextProperty name;


	if(!text || size == 0)
		return 0;

	text[0] = '\0';

	if(!XGetTextProperty(dwm.dpy, w, &name, atom) || !name.nitems)
		return 0;

	if(name.encoding == XA_STRING){
		strncpy(text, (char*)name.value, size - 1);
	}
	else if(XmbTextPropertyToTextList(dwm.dpy, &name, &list, &n) >= Success && n > 0 && *list){
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);

	return 1;
}

Atom atoms_atom_prop(client_t *c, Atom prop){
	int di;
	unsigned long dl;
	unsigned char *p = NULL;
	Atom da, atom = None;


	if(XGetWindowProperty(dwm.dpy, c->win, prop, 0L, sizeof atom, False, XA_ATOM, &da, &di, &dl, &dl, &p) == Success && p){
		atom = *(Atom*)p;
		XFree(p);
	}

	return atom;
}
