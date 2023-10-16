#include <config/config.h>
#include <unistd.h>
#include <signal.h>
#include <core/dwm.h>
#include <core/scheme.h>
#include <utils/log.h>
#include <commands.h>


/* global functions */
void cmd_spawn(cmd_arg_t *arg){
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

void cmd_dmenu_run(cmd_arg_t *arg){
	char const *dmenu[] = {
		"dmenurun",
		"-fn", CONFIG_FONT,
		"-nf", dwm.gfx->schemes[SCM_NORM].names->fg,
		"-nb", dwm.gfx->schemes[SCM_NORM].names->bg,
		"-sf", dwm.gfx->schemes[SCM_FOCUS].names->fg,
		"-sb", dwm.gfx->schemes[SCM_FOCUS].names->bg,
		0x0
	};


	cmd_spawn(&(cmd_arg_t){ .v = dmenu });
}
