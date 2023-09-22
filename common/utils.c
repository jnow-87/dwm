#include <dwm.h>
#include <stdlib.h>


/* global functions */
void *ecalloc(size_t nmemb, size_t size){
	void *p;


	if(!(p = calloc(nmemb, size)))
		die("calloc:");

	return p;
}
