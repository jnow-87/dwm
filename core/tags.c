#include <core/clientstack.h>
#include <core/layout.h>
#include <core/statusbar.h>
#include <config.h>


/* macros */
#define TAGMASK ((1 << ntags) - 1)


/* global functions */
void tags_set(unsigned int *tags, unsigned int v){
	v &= TAGMASK;

	if(v == 0 || *tags == v)
		return;

	*tags = v;

	// TODO this should be moved to dwm level
	clientstack_refocus();
	layout_arrange();
	statusbar_update();
}

void tags_toggle(unsigned int *tags, unsigned int v){
	tags_set(tags, v ^ *tags);
}
