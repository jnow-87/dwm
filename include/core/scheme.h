#ifndef SCHEME_H
#define SCHEME_H


#include <utils/utils.h>


/* macros */
#define SCHEME(_id, _fg, _bg, _border) \
	static color_scheme_t const scheme_##_id \
		linker_array("schemes") noreorder = { \
			.id = _id, \
			.fg = _fg, \
			.bg = _bg, \
			.border = _border, \
		}


/* types */
typedef enum{
	SCM_NORM,
	SCM_FOCUS,
	SCM_STATUS,
	SCM_SPACER_NORM,
	SCM_SPACER_STATUS,
	NSCMS
} scheme_id_t;

typedef struct{
	scheme_id_t id;

	char const *fg,
			   *bg,
			   *border;
} color_scheme_t;


/* external variables */
extern color_scheme_t __start_schemes[],
					  __stop_schemes[];


#endif // SCHEME_H
