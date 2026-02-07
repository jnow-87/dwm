#ifndef TAGS_H
#define TAGS_H


#include <stddef.h>


/* prototypes */
void tags_set(unsigned int *tags, unsigned int v);
void tags_toggle(unsigned int *tags, unsigned int v);
char *tags_name(unsigned int xtags, char *name, size_t n);


#endif // TAGS_H
