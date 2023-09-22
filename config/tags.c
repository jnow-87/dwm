#include <utils.h>


/* global variables */
char const *tags[] = { "💻", "💻", "🗇", "📟" };
unsigned int ntags = LENGTH(tags);

static_assert(LENGTH(tags) < 32, "too many tags, number of tags must fit into an unsigned int bit array");
