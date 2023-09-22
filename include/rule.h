#ifndef RULE_H
#define RULE_H


/* types */
typedef struct{
	char const *class;
	char const *instance;
	char const *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} rule_t;


#endif // RULE_H
