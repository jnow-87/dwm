#include <layout.h>
#include <utils.h>


/* global variables */
layout_t const layouts[] = {
	/* symbol     arrange function */
	{ "☯", NULL },
	{ "⚏", tile },
	{ "❍", monocle },
};

unsigned int const nlayouts = LENGTH(layouts);
