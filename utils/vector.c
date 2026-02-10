#include <stdlib.h>
#include <string.h>
#include <utils/vector.h>


/* global functions */
int vector_init(vector_t *v, size_t dt_size, size_t capa){
	v->buf = malloc(dt_size * capa);

	if(v->buf == 0x0)
		return -1;

	v->capacity = capa;
	v->size = 0;
	v->dt_size = dt_size;

	return 0;
}

void vector_destroy(vector_t *v){
	free(v->buf);

	v->capacity = 0;
	v->size = 0;
}

int vector_cp(vector_t *dest, vector_t *src){
	*dest = *src;
	dest->buf = malloc(src->capacity * src->dt_size);

	if(dest->buf == 0x0)
		return -1;

	memcpy(dest->buf, src->buf, src->size * src->dt_size);

	return 0;
}

void vector_mv(vector_t *dest, vector_t *src){
	*dest = *src;
	*src = VECTOR_INITIALISER(dest->dt_size);
}

int vector_add(vector_t *v, void *buf){
	void *t;


	if(v->size >= v->capacity){
		v->capacity += 8;
		t = malloc(v->capacity * v->dt_size);

		if(t == 0x0)
			return -1;

		memcpy(t, v->buf, v->size * v->dt_size);
		free(v->buf);
		v->buf = t;
	}

	memcpy(v->buf + v->size * v->dt_size, buf, v->dt_size);
	v->size++;

	return 0;
}

void vector_rm(vector_t *v, size_t idx){
	if(idx >= v->size)
		return;

	memcpy(v->buf + idx * v->dt_size, v->buf + (idx + 1) * v->dt_size, (v->size - idx - 1) * v->dt_size);
	v->size--;
}

void *vector_get(vector_t *v, size_t idx){
	if(idx >= v->size)
		return 0x0;

	return v->buf + idx * v->dt_size;
}
