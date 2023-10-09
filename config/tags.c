#include <stddef.h>
#include <utils/utils.h>


/* global variables */
char const *tags[] = { "💻", "💻", "🗇", "📟" };
size_t ntags = LENGTH(tags);

static_assert(LENGTH(tags) < 32, "too many tags, number of tags must fit into an unsigned int bit array");
