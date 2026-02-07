#ifndef SCHEME_H
#define SCHEME_H


#include <utils/utils.h>


/* macros */
#define SCHEME_INITIALISER() (color_scheme_t){ \
	.id = NSCMS, \
	.fg = 0x0, \
	.bg = 0x0, \
	.border = 0x0, \
}

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
	SCM_SPACER_INTRA,
	SCM_SPACER_STATUS,
	NSCMS
} scheme_id_t;

typedef struct{
	scheme_id_t id;

	char *fg,
		 *bg,
		 *border;
} color_scheme_t;


/* external variables */
extern color_scheme_t __start_schemes[],
					  __stop_schemes[];


/* prototypes */
int schemes_verify(void);
int scheme_verify(color_scheme_t *scm);


#endif // SCHEME_H
