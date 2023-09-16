#include <utils.h>
#include <layout.h>


/* global variables */
const layout_t layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
};

const unsigned int nlayouts = LENGTH(layouts);
