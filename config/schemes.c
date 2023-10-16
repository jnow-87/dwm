#include <core/scheme.h>


/* macros */
#define FOCUS_FG		"#eeeeee"
#define FOCUS_BG		"#005f5f"
#define FOCUS_BORDER	"#005f5f"
#define NORM_FG			"#bbbbbb"
#define NORM_BG			"#222222"
#define NORM_BORDER		"#005f5f"


/* color schemes */
SCHEME(SCM_NORM, NORM_FG, NORM_BG, NORM_BORDER);
SCHEME(SCM_FOCUS, FOCUS_FG, FOCUS_BG, FOCUS_BORDER);
SCHEME(SCM_SPACER, NORM_BG, FOCUS_BG, FOCUS_BORDER);
