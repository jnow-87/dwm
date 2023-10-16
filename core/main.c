#include <version.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <core/dwm.h>


/* global functions */
int main(int argc, char *argv[]){
	if(argc == 2 && !strcmp("-v", argv[1]))
		return printf("dwm-" VERSION) != 0;

	if(argc != 1)
		return fprintf(stderr, "usage: dwm [-v]");

	if(dwm_setup() != 0)
		fprintf(stderr, "setup failed\n");

	dwm_run();
	dwm_cleanup();

	if(dwm.state == DWM_RESTART)
		execvp(argv[0], argv);

	return (dwm.state != DWM_SHUTDOWN);
}
