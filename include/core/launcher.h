#ifndef LAUNCHER_H
#define LAUNCHER_H


#include <utils/utils.h>


/* macros */
#define LAUNCHER_ITEM_N(n, _name, _cmd) \
	static launcher_item_t launcher_item_##n \
		linker_array("launcher_items") noreorder = { \
			.name = _name, \
			.cmd = _cmd, \
		}

#define LAUNCHER_ITEM(name, cmd)		UNIQUE(LAUNCHER_ITEM_N, __COUNTER__, name, cmd)


/* types */
typedef struct{
	char const *name,
			   *cmd;
} launcher_item_t;


/* external variables */
extern launcher_item_t __start_launcher_items[],
					   __stop_launcher_items[];


#endif // LAUNCHER_H
