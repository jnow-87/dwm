#ifndef TAGS_H
#define TAGS_H


#include <utils/utils.h>


/* macros */
#define TAG_N(n, s) 	static char const *tag_##n linker_array("tags") noreorder = s
#define TAG(s)			UNIQUE(TAG_N, __COUNTER__, s)


/* prototypes */
void tags_set(unsigned int *tags, unsigned int v);
void tags_toggle(unsigned int *tags, unsigned int v);


/* external variables */
extern char *__start_tags[],
			*__stop_tags[];


#endif // TAGS_H
