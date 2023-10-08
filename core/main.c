#include <version.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
#include <core/dwm.h>
#include <utils/log.h>


/* global variables */
dwm_t dwm = {
	.stack = 0x0,
	.layout = layouts + 0,
	.state = DWM_ERROR,
	.numlock_mask = 0,
	.tag_mask = 1,
};


/* global functions */
int main(int argc, char *argv[]){
	if(argc == 2 && !strcmp("-v", argv[1]))
		return printf("dwm-" VERSION) != 0;

	if(argc != 1)
		return fprintf(stderr, "usage: dwm [-v]");

	if(dwm_setup() != 0)
		ERROR("setup failed\n");

	dwm_run();
	dwm_cleanup();

	if(dwm.state == DWM_RESTART)
		execvp(argv[0], argv);

	return (dwm.state != DWM_SHUTDOWN);
}
