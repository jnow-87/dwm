#include <core/scheme.h>


/* macros */
#define BORDER			"#005f5f"
#define FG				"#eeeeee"

#define FOCUS_BG		"#005f5f"
#define NORM_BG			"#222222"
#define STATUS_BG		"#00875f"


/* color schemes */
SCHEME(SCM_NORM, FG, NORM_BG, BORDER);
SCHEME(SCM_FOCUS, FG, FOCUS_BG, BORDER);
SCHEME(SCM_STATUS, FG, STATUS_BG, BORDER);

SCHEME(SCM_SPACER_NORM, NORM_BG, STATUS_BG, BORDER);
SCHEME(SCM_SPACER_STATUS, STATUS_BG, FOCUS_BG, BORDER);
