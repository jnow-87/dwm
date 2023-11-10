#include <stddef.h>
#include <core/tags.h>


/* macros */
#define TAGMASK		((1 << (__stop_tags - __start_tags)) - 1)


/* global functions */
void tags_set(unsigned int *tags, unsigned int v){
	v &= TAGMASK;

	if(v == 0 || *tags == v)
		return;

	*tags = v;
}

void tags_toggle(unsigned int *tags, unsigned int v){
	tags_set(tags, v ^ *tags);
}

size_t tags_selected(unsigned int tags){
	size_t n = 0;


	for(size_t i=0; i<__stop_tags-__start_tags; i++){
		if(tags & (1 << i))
			n++;
	}

	return n;
}
