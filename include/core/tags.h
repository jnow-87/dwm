#ifndef TAGS_H
#define TAGS_H


#include <stddef.h>
#include <utils/utils.h>


/* macros */
#define TAG_N(n, s) 	static char const *tag_##n linker_array("tags") noreorder = s
#define TAG(s)			UNIQUE(TAG_N, __COUNTER__, s)


/* prototypes */
void tags_set(unsigned int *tags, unsigned int v);
void tags_toggle(unsigned int *tags, unsigned int v);
char *tags_name(unsigned int xtags, char *name, size_t n);


/* external variables */
extern char *__start_tags[],
			*__stop_tags[];


#endif // TAGS_H
