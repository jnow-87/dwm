#include <string.h>
#include <stdlib.h>
#include <utils/string.h>


/* global functions */
char *stralloc(char const *s){
	return strnalloc(s, strlen(s));
}

char *strnalloc(char const *s, size_t n){
	char *x;


	x = (char*)malloc(n + 1);

	if(x == 0x0)
		return 0x0;

	memcpy(x, s, n);
	x[n] = 0;

	return x;
}
