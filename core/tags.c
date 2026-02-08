#include <config/config.h>
#include <stddef.h>
#include <stdio.h>
#include <core/tags.h>
#include <rc.h>


/* macros */
#define TAGMASK		((1 << (rc.tags.size)) - 1)


/* local/static prototypes */
static size_t ntags(unsigned int tags);
static size_t first(unsigned int tags);


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

char *tags_name(unsigned int tags, char *name, size_t n){
	size_t i;
	char **tag;


	i = ntags(tags);

	if(i == 1){
		tag = vector_get(&rc.tags, first(tags));
		snprintf(name, n, "%s", (tag != 0x0) ? *tag : "none");
	}
	else
		snprintf(name, n, "%s [%zu]", rc.statusbar.icon_tags_multi, i);

	return name;
}


/* local functions */
static size_t ntags(unsigned int tags){
	size_t n = 0;


	for(size_t i=0; i<rc.tags.size; i++){
		if(tags & (1 << i))
			n++;
	}

	return n;
}

static size_t first(unsigned int tags){
	for(size_t i=0; i<rc.tags.size; i++){
		if(tags & (1 << i))
			return i;
	}

	return 0;
}
