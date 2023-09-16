#include <stdlib.h>
#include <dwm.h>


void *ecalloc(size_t nmemb, size_t size){
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}


