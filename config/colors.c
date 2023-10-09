#include <config/config.h>
#include <stddef.h>
#include <xlib/gfx.h>
#include <utils/utils.h>


/* global variables */
char const *colors[NSCMS][3] = {
	/*               fg         bg         border   */
	[SCM_NORM] = { CONFIG_COL_INACT_FG, CONFIG_COL_INACT_BG, CONFIG_COL_INACT_BORDER },
	[SCM_FOCUS] = { CONFIG_COL_ACT_FG, CONFIG_COL_ACT_BG, CONFIG_COL_ACT_BORDER },
	[SCM_SPACER] = { CONFIG_COL_INACT_BG, CONFIG_COL_ACT_BG, CONFIG_COL_ACT_BORDER },
};
