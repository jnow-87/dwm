#ifndef STACK_H
#define STACK_H


#include <utils/list.h>


/* macros */
#define stack_top(top)({ \
	LIST_TYPE_COMPAT2(top); \
	\
	top; \
})

#define stack_push(top, el) \
	list_add_head(top, el)

#define stack_pop(top)({ \
	typeof(top) _el = list_first(top); \
	\
	if(_el != 0x0) \
		list_rm(top, _el); \
	\
	_el; \
})

#define stack_raise(top, el){ \
	list_rm(top, el); \
	list_add_head(top, el); \
}


#endif // STACK_H
