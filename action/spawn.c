#include <unistd.h>
#include <signal.h>
#include <core/dwm.h>
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
	sigaction(SIGCHLD, &sa, NULL);

	execvp(((char **)arg->v)[0], (char **)arg->v);
	dwm_die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
}
