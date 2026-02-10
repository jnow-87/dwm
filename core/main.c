#include <unistd.h>
#include <core/dwm.h>


/* global functions */
int main(int argc, char *argv[]){
	if(dwm_setup() != 0)
		return 1;

	dwm_run();
	dwm_cleanup();

	if(dwm.state == DWM_RESTART)
		execvp(argv[0], argv);

	return (dwm.state != DWM_SHUTDOWN);
}
