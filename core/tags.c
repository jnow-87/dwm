#include <core/clientstack.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <core/tags.h>


/* macros */
#define TAGMASK		((1 << (__stop_tags - __start_tags)) - 1)


/* global functions */
void tags_set(unsigned int *tags, unsigned int v){
	v &= TAGMASK;

	if(v == 0 || *tags == v)
		return;

	*tags = v;

	layout_arrange();
	clientstack_refocus();
	statusbar_update();
}

void tags_toggle(unsigned int *tags, unsigned int v){
	tags_set(tags, v ^ *tags);
}
