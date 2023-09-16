#ifndef UTILS_H
#define UTILS_H


#include <stddef.h>


/* macros */
#define LENGTH(X)	(sizeof X / sizeof X[0])
#define MAX(A, B)	((A) > (B) ? (A) : (B))
#define MIN(A, B)	((A) < (B) ? (A) : (B))

#define BETWEEN(X, A, B)	((A) <= (X) && (X) <= (B))

#define INTERSECT(x,y,w,h,m) \
	  (MAX(0, MIN((x)+(w),(m)->wx+(m)->ww) - MAX((x),(m)->wx)) \
	* MAX(0, MIN((y)+(h),(m)->wy+(m)->wh) - MAX((y),(m)->wy)))


/* prototypes */
void *ecalloc(size_t nmemb, size_t size);


#endif // UTILS_H
