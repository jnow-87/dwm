#include <version.h>
#include <unistd.h>
#include <string.h>
#include <config.h>
#include <core/dwm.h>


/* global variables */
dwm_t dwm = {
	.stack = 0x0,
	.layout = layouts + 0,
	.running = 1,
	.numlock_mask = 0,
	.tag_mask = 1,
};


/* global functions */
int main(int argc, char *argv[]){
	if(argc == 2 && !strcmp("-v", argv[1]))
		dwm_die("dwm-" VERSION);

	if(argc != 1)
		dwm_die("usage: dwm [-v]");

	if(dwm_setup() != 0)
		dwm_die("dwm_setup failed\n");

	dwm_run();
	dwm_cleanup();

	// restart
	if(dwm.running < 0)
		execvp(argv[0], argv);

	return EXIT_SUCCESS;
}
