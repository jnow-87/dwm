#include <config/config.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <core/dwm.h>
#include <core/scheme.h>
#include <utils/log.h>
#include <commands.h>


/* local/static prototypes */
static void exec(char * const argv[]);


/* global functions */
void cmd_spawn(cmd_arg_t const *arg){
	if(fork() != 0)
		return;

	exec(arg->v);
}

void cmd_dmenu_run(cmd_arg_t const *arg){
	char const *dmenu[] = {
		"dmenurun",
		"-fn", CONFIG_FONT,
		"-m", "0",
		"-nf", dwm.gfx->schemes[SCM_NORM].names->fg,
		"-nb", dwm.gfx->schemes[SCM_NORM].names->bg,
		"-sf", dwm.gfx->schemes[SCM_FOCUS].names->fg,
		"-sb", dwm.gfx->schemes[SCM_FOCUS].names->bg,
		0x0
	};
	pid_t pid;


	// NOTE
	// 	in contrast to cmd_spawn() wait for dmenurun to return
	// 	this avoids dwm taking any actions, such as updates to
	// 	the statusbar status, which would raise the statusbar
	// 	window above dmenu in case both windows occupy the same
	// 	screen area
	pid = fork();

	switch(pid){
	case 0:		exec((char **)dmenu); break;
	case -1:	return;
	default:	waitpid(pid, 0x0, 0); break;
	}
}


/* local functions */
static void exec(char * const argv[]){
	struct sigaction sa;


	if(dwm.dpy)
		close(ConnectionNumber(dwm.dpy));

	setsid();

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = SIG_DFL;
	sigaction(SIGCHLD, &sa, 0x0);

	execvp(argv[0], argv);
	ERROR("spawning %s\n", argv[0]);
}
