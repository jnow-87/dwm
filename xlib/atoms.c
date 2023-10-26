#include <string.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <core/dwm.h>


/* global functions */
int atoms_text_prop(Window win, Atom atom, char *text, unsigned int size){
	char **list = 0x0;
	int n;
	XTextProperty name;


	if(!text || size == 0)
		return -1;

	text[0] = '\0';

	if(!XGetTextProperty(dwm.dpy, win, &name, atom) || !name.nitems)
		return -1;

	if(name.encoding == XA_STRING){
		strncpy(text, (char*)name.value, size - 1);
	}
	else if(XmbTextPropertyToTextList(dwm.dpy, &name, &list, &n) >= Success && n > 0 && *list){
		strncpy(text, *list, size - 1);
		XFreeStringList(list);
	}

	text[size - 1] = '\0';
	XFree(name.value);

	return 0;
}

int atoms_text_prop_set(Window win, Atom atom, char *text){
	XTextProperty prop;


	if(Xutf8TextListToTextProperty(dwm.dpy, &text, 1, XStdICCTextStyle, &prop) != Success)
		return -1;

	XSetTextProperty(dwm.dpy, win, &prop, atom);

	return 0;
}

void atoms_netatom_append(net_atom_t atom, unsigned char *value){
	XChangeProperty(dwm.dpy, dwm.root, dwm.netatom[atom], XA_WINDOW, 32, PropModeAppend, value, 1);
}

void atoms_netatom_delete(net_atom_t atom){
	XDeleteProperty(dwm.dpy, dwm.root, dwm.netatom[atom]);
}
