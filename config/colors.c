#include <colors.h>
#include <config/config.h>
#include <utils.h>


/* global variables */
char const *colors[][3] = {
	/*               fg         bg         border   */
	[SchemeNorm] = { CONFIG_COL_INACT_FG, CONFIG_COL_INACT_BG, CONFIG_COL_INACT_BORDER },
	[SchemeSel] = { CONFIG_COL_ACT_FG,	  CONFIG_COL_ACT_BG,	 CONFIG_COL_ACT_BORDER },
};

unsigned int ncolors = LENGTH(colors);
