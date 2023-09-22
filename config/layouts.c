#include <layout.h>
#include <utils.h>


/* global variables */
layout_t const layouts[] = {
	/* symbol     arrange function */
	{ "[]=", tile }, /* first entry is default */
	{ "><>", NULL }, /* no layout function means floating behavior */
	{ "[M]", monocle },
};

unsigned int const nlayouts = LENGTH(layouts);
