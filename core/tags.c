#include <dwm.h>
#include <config.h>


/* global functions */
void tags_set(unsigned int *tags, unsigned int v){
	v &= TAGMASK;

	if(v == 0 || *tags == v)
		return;

	*tags = v;

	// TODO this should be moved to dwm level
	focus(NULL);
	arrange(dwm.mons);
	statusbar_draw();
}

void tags_toggle(unsigned int *tags, unsigned int v){
	tags_set(tags, v ^ *tags);
}
