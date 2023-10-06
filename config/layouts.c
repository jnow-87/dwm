#include <core/layout.h>
#include <utils/math.h>


/* global variables */
layout_t const layouts[] = {
	/* symbol     layout_arrange function */
	{ "☯", NULL },
	{ "⚏", layout_tile },
	{ "❍", layout_monocle },
};

unsigned int const nlayouts = LENGTH(layouts);
