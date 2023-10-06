#ifndef ATOMS_H
#define ATOMS_H


/* prototypes */
int atoms_text_prop(Window w, Atom atom, char *text, unsigned int size);
Atom atoms_atom_prop(client_t *c, Atom prop);


#endif // ATOMS_H
