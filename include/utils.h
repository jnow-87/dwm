#ifndef UTILS_H
#define UTILS_H


#include <stddef.h>


/* macros */
#define LENGTH(X)	(sizeof X / sizeof X[0])
#define MAX(A, B)	((A) > (B) ? (A) : (B))
#define MIN(A, B)	((A) < (B) ? (A) : (B))

#define BETWEEN(X, A, B)	((A) <= (X) && (X) <= (B))


/* prototypes */
void *ecalloc(size_t nmemb, size_t size);


#endif // UTILS_H
