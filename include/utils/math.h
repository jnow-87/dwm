#ifndef MATH_H
#define MATH_H


/* macros */
#define CMP(x, y, op)({ \
	typeof(x) _x = x; \
	typeof(y) _y = y; \
	\
	\
	(_x op _y) ? _x : _y; \
})

#define MIN(x, y)	CMP(x, y, <)
#define MAX(x, y)	CMP(x, y, >)


#endif // MATH_H
