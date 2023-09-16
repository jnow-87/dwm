#ifndef RULE_H
#define RULE_H


/* types */
typedef struct {
	const char *class;
	const char *instance;
	const char *title;
	unsigned int tags;
	int isfloating;
	int monitor;
} rule_t;


#endif // RULE_H
