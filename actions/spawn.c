#include <unistd.h>
#include <signal.h>
#include <core/dwm.h>
#include <utils/log.h>
#include <actions.h>


/* global functions */
void action_spawn(action_arg_t const *arg){
	struct sigaction sa;


	if(fork() != 0)
		return;

	if(dwm.dpy)
		close(ConnectionNumber(dwm.dpy));

	setsid();

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	sigaction(SIGCHLD, &sa, 0x0);

	execvp(((char **)arg->v)[0], (char **)arg->v);
	ERROR("spawning %s\n", ((char **)arg->v)[0]);
}
