#include <config/config.h>
#include <stddef.h>
#include <core/scheme.h>
#include <utils/utils.h>


/* global variables */
char const *colors[][3] = {
	/*               fg         bg         border   */
	[SchemeNorm] = { CONFIG_COL_INACT_FG, CONFIG_COL_INACT_BG, CONFIG_COL_INACT_BORDER },
	[SchemeSel] = { CONFIG_COL_ACT_FG, CONFIG_COL_ACT_BG, CONFIG_COL_ACT_BORDER },
	[SchemeSpacer] = { CONFIG_COL_INACT_BG, CONFIG_COL_ACT_BG, CONFIG_COL_ACT_BORDER },
};

size_t ncolors = LENGTH(colors);
