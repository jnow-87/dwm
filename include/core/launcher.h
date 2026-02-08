#ifndef LAUNCHER_H
#define LAUNCHER_H


#include <utils/vector.h>


/* types */
typedef struct{
	char const *name;
	vector_t argv;
} launcher_item_t;


/* prototypes */
int launcher_init(launcher_item_t *item, char const *name, char *cmdline);
void launcher_cleanup(launcher_item_t *item);


#endif // LAUNCHER_H
