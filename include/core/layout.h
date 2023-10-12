#ifndef LAYOUT_H
#define LAYOUT_H


#include <core/client.h>
#include <core/monitor.h>
#include <xlib/window.h>
#include <utils/utils.h>


/* macros */
#define LAYOUT(_symbol, _arrange) \
	static layout_t const layout_##_arrange \
		linker_array("layouts") noreorder = { \
			.symbol = _symbol, \
			.arrange = _arrange, \
		}


/* types */
typedef struct{
	char const *symbol;
	void (*arrange)(void);
} layout_t;


/* prototypes */
client_t *layout_next_tiled(client_t *c, monitor_t *m);
void layout_arrange(void);


/* external variables */
extern layout_t __start_layouts[],
				__stop_layouts[];


#endif // LAYOUT_H
