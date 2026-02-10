#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <core/dwm.h>
#include <utils/log.h>


/* global functions */
int exec(char * const argv[]){
	pid_t pid;
	struct sigaction sa;


	pid = fork();

	if(pid != 0)
		return pid;

	if(dwm.dpy)
		close(ConnectionNumber(dwm.dpy));

	setsid();

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	sigaction(SIGCHLD, &sa, 0x0);

	execvp(argv[0], argv);
	ERROR("spawning %s\n", argv[0]);

	return 0;
}
