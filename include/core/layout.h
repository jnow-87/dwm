#ifndef LAYOUT_H
#define LAYOUT_H


#include <core/client.h>
#include <core/monitor.h>
#include <xlib/window.h>
#include <utils/utils.h>


/* macros */
#define LAYOUT_INITIALISER(_name, _symbol, _arrange) (layout_t){ \
	.name = _name, \
	.symbol = _symbol, \
	.arrange = _arrange, \
}

#define LAYOUT_N(_n, _name, _symbol, _arrange) \
	static layout_t const layout_##_n \
		linker_array("layouts") noreorder = LAYOUT_INITIALISER( \
			_symbol " " _name, \
			_symbol, \
			_arrange \
		)

#define LAYOUT(name, symbol, arrange)	UNIQUE(LAYOUT_N, __COUNTER__, name, symbol, arrange)


/* types */
typedef struct{
	char const *name,
			   *symbol;
	void (*arrange)(void);
} layout_t;


/* prototypes */
client_t *layout_next_tiled(client_t *c, monitor_t *m);
void layout_arrange(void);


/* external variables */
extern layout_t __start_layouts[],
				__stop_layouts[];


#endif // LAYOUT_H
