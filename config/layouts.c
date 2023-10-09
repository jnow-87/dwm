#include <stddef.h>
#include <core/layout.h>
#include <utils/utils.h>


/* global variables */
layout_t const layouts[] = {
	/* symbol     layout_arrange function */
	{ "☯", 0x0 },
	{ "⚏", layout_tile },
	{ "❍", layout_monocle },
};

size_t nlayouts = LENGTH(layouts);
