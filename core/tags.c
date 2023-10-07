#include <core/dwm.h>
#include <core/stack.h>
#include <config.h>


/* global functions */
void tags_set(unsigned int *tags, unsigned int v){
	v &= TAGMASK;

	if(v == 0 || *tags == v)
		return;

	*tags = v;

	// TODO this should be moved to dwm level
	client_refocus();
	layout_arrange();
	statusbar_update();
}

void tags_toggle(unsigned int *tags, unsigned int v){
	tags_set(tags, v ^ *tags);
}
