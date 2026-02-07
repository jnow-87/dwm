#ifndef VECTOR_H
#define VECTOR_H


#include <stddef.h>


/* macros */
#define VECTOR_INITIALISER(_dt_size) (vector_t){ \
	.buf = 0x0, \
	.dt_size = (_dt_size), \
	.capacity = 0, \
	.size = 0, \
}

#define vector_for_each(v, p) \
	for(p=(v)->buf; p<(typeof(p))((v)->buf+(v)->dt_size*(v)->size); p++)


/* types */
typedef struct{
	void *buf;

	size_t dt_size;

	size_t capacity,
		   size;
} vector_t;


/* prototypes */
int vector_init(vector_t *v, size_t dt_size, size_t capa);
void vector_destroy(vector_t *v);

int vector_cp(vector_t *dest, vector_t *src);
void vector_mv(vector_t *dest, vector_t *src);
int vector_add(vector_t *v, void *buf);
void vector_rm(vector_t *v, size_t idx);

void *vector_get(vector_t *v, size_t idx);


#endif // VECTOR_H
