#ifndef LIST_H
#define LIST_H


#include <stdbool.h>
#include <stddef.h>


/* macros */
#define offsetofvar(var, member)	offsetof(typeof(var), member)

/**
 * \brief	check if the next and prev pointer of the given variable have the
 * 			same offset as the next and prev pointer within list2_t
 */
#define LIST_TYPE_COMPAT2(var) \
	_Static_assert(offsetof(list2_t, prev) == offsetofvar(var, prev), \
		#var " cannot be used as list, since the offset of the prev pointer does not match list2_t"); \
	_Static_assert(offsetof(list2_t, next) == offsetofvar(var, next), \
		#var " cannot be used as list, since the offset of the next pointer does not match list2_t");


// list interface
#define list_init(head){ \
	LIST_TYPE_COMPAT2(*head); \
	\
	_list2_init((list2_t*)head); \
}

#define list_add_head(head, el){ \
	LIST_TYPE_COMPAT2(*head); \
	LIST_TYPE_COMPAT2(*el); \
	\
	_list2_add_head((list2_t**)(&(head)), (list2_t*)(el)); \
}

#define list_add_head_safe(head, el, mtx) \
	LOCK_SECTION(mtx, list_add_head(head, el))

#define list_add_tail(head, el){ \
	LIST_TYPE_COMPAT2(*head); \
	LIST_TYPE_COMPAT2(*el); \
	\
	_list2_add_tail((list2_t**)(&(head)), (list2_t*)(el)); \
}

#define list_add_tail_safe(head, el, mtx) \
	LOCK_SECTION(mtx, list_add_tail(head, el))

#define list_add_in(el, front, back){ \
	LIST_TYPE_COMPAT2(*el); \
	LIST_TYPE_COMPAT2(*front); \
	LIST_TYPE_COMPAT2(*back); \
	\
	_list2_add_in((list2_t*)(el), (list2_t*)(front), (list2_t*)(back)); \
}

#define list_add_in_safe(el, front, back, mtx) \
	LOCK_SECTION(mtx, list_add_in(el, front, back))

#define list_replace(head, old, new){ \
	LIST_TYPE_COMPAT2(*head); \
	LIST_TYPE_COMPAT2(*old); \
	LIST_TYPE_COMPAT2(*new); \
	\
	_list2_replace((list2_t**)(&(head)), (list2_t*)(old), (list2_t*)(new)); \
}

#define list_replace_safe(head, old, new, mtx) \
	LOCK_SECTION(mtx, list_replace(head, old, new))

#define list_rm(head, el){ \
	LIST_TYPE_COMPAT2(*head); \
	LIST_TYPE_COMPAT2(*el); \
	\
	_list2_rm((list2_t**)(&(head)), (list2_t*)(el)); \
}

#define list_rm_safe(head, el, mtx) \
	LOCK_SECTION(mtx, list_rm(head, el))

#define list_first(head) (head)

#define list_first_safe(head, mtx)({ \
	typeof(head) _el; \
	\
	\
	LOCK_SECTION(mtx, _el = list_first(head)); \
	_el; \
})

#define list_last(head)({ \
	LIST_TYPE_COMPAT2(*head); \
	\
	(typeof(head))_list2_last((list2_t*)(head)); \
})

#define list_last_safe(head, mtx)({ \
	typeof(head) _el; \
	\
	\
	LOCK_SECTION(mtx, _el = list_last(head)); \
	_el; \
})

#define list_contains(head, el)({ \
	LIST_TYPE_COMPAT2(*head); \
	LIST_TYPE_COMPAT2(*el); \
	\
	_list2_contains((list2_t*)(head), (list2_t*)(el)); \
})

#define list_contains_safe(head, el, mtx)({ \
	bool _r; \
	\
	\
	LOCK_SECTION(mtx, _r = list_contains(head, el)); \
	_r; \
})

#define list_find(head, member, value)({ \
	typeof(value) _value = value; \
	\
	\
	LIST_TYPE_COMPAT2(*head); \
	\
	(typeof(head))_list2_find((list2_t*)(head), offsetofvar(*(head), member), (void*)(&(_value)), sizeof(value)); \
})

#define list_find_safe(head, member, value, mtx)({ \
	typeof(head) _el; \
	\
	\
	LOCK_SECTION(mtx, _el = list_find(head, member, value)); \
	_el; \
})

#define list_find_str(head, member, str)({ \
	LIST_TYPE_COMPAT2(*head); \
	\
	list_find_strn(head, member, str, 0); \
})

#define list_find_str_safe(head, member, str, mtx)({ \
	typeof(head) _el; \
	\
	\
	LOCK_SECTION(mtx, _el = list_find_str(head, member, str)); \
	_el; \
})

#define list_find_strn(head, member, str, n)({ \
	typeof(head) _head = head; \
	\
	\
	LIST_TYPE_COMPAT2(*head); \
	\
	(typeof(head))_list2_find_str((list2_t*)_head, offsetofvar(*_head, member), str, n, ((void*)(_head->member) == (void*)(&(_head->member)) ? true : false)); \
})

#define list_find_strn_safe(head, member, str, n, mtx)({ \
	typeof(head) _el; \
	\
	\
	LOCK_SECTION(mtx, _el = list_find_strn(head, member, str, n)); \
	_el; \
})

#define list_empty(head) (((head) == 0x0) ? true : false)

#define list_empty_safe(head, mtx)({ \
	int _r; \
	\
	\
	LOCK_SECTION(mtx, _r = list_empty(head)); \
	_r; \
})

#define list_for_each(head, el) \
	__list_for_each(head, el, next)

#define __list_for_each(head, el, next_name) \
	el = head; \
	\
	for(typeof(head) next=((head) == 0x0 ? 0x0 : (head)->next_name); (el)!=0x0; (el)=(next_name), next=(next == 0x0 ? 0 : next->next_name))


/* types */
typedef struct list2_t{
	struct list2_t *prev,
				   *next;
} list2_t;


/* prototypes */
// double-linked list
void _list2_init(list2_t *head);
void _list2_add_head(list2_t **head, list2_t *el);
void _list2_add_tail(list2_t **head, list2_t *el);
void _list2_add_in(list2_t *el, list2_t *front, list2_t *back);
void _list2_replace(list2_t **head, list2_t *old, list2_t *new);
void _list2_rm(list2_t **head, list2_t *el);
list2_t *_list2_last(list2_t *head);
bool _list2_contains(list2_t *head, list2_t *el);
list2_t *_list2_find(list2_t *head, size_t mem_offset, void const *ref, size_t n);
list2_t *_list2_find_str(list2_t *head, size_t mem_offset, char const *ref, size_t n, bool is_array);


#endif // LIST_H
