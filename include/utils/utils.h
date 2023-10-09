#ifndef MATH_H
#define MATH_H


/* macros */
#define LENGTH(X)	(sizeof X / sizeof X[0])
#define MAX(A, B)	((A) > (B) ? (A) : (B))
#define MIN(A, B)	((A) < (B) ? (A) : (B))

#define section(sec)			__attribute__((section(sec)))
#define align(base)				__attribute__((aligned(base)))
#define used					__attribute__((used))
#define noreorder				__attribute__((no_reorder))
#define linker_array(sec)		section(sec) align(1) used

#define UNIQUE(impl, num, ...)	impl(num, __VA_ARGS__)

#define config_for_each(obj, el) \
	for(el=__start_##obj; el!=__stop_##obj; el++)


#endif // MATH_H
