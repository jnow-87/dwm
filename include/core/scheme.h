#ifndef SCHEME_H
#define SCHEME_H


/* macros */
#define SCHEME_INITIALISER() (color_scheme_t){ \
	.id = NSCMS, \
	.fg = 0x0, \
	.bg = 0x0, \
	.border = 0x0, \
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


/* prototypes */
int schemes_verify(void);
int scheme_verify(color_scheme_t *scm);


#endif // SCHEME_H
