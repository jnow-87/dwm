#ifndef LAYOUT_H
#define LAYOUT_H


#include <client.h>
#include <monitor.h>


/* types */
typedef struct layout_t{
	char const *symbol;
	void (*arrange)(void);
} layout_t;


/* prototypes */
client_t *nexttiled(client_t *c);
void arrange(void);

void tile(void);
void monocle(void);


#endif // LAYOUT_H
