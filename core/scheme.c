#include <core/scheme.h>
#include <utils/log.h>
#include <utils/vector.h>
#include <rc.h>


/* global functions */
int schemes_verify(void){
	unsigned int ids = 0;
	color_scheme_t *scm;


	vector_for_each(&rc.schemes, scm)
		ids |= (1 << scm->id);

	for(size_t i=0; i<NSCMS; i++){
		if((ids & (1 << i)) == 0)
			return ERROR("missing color scheme\n");
	}

	return 0;
}

int scheme_verify(color_scheme_t *scm){
	if(scm->fg == 0x0 || scm->bg == 0x0 || scm->border == 0x0)
		return ERROR("missing color-scheme component\n");

	return 0;
}
