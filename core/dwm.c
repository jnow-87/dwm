/* See LICENSE file for copyright and license details.
 *
 * dynamic window manager is designed like any other X client as well. It is
 * driven through handling X events. In contrast to other X clients, a window
 * manager selects for SubstructureRedirectMask on the root window, to receive
 * events about window (dis-)appearance. Only one X connection at a time is
 * allowed to select for this event mask.
 *
 * Each child of the root window is called a client, except windows which have
 * set the override_redirect flag. Clients are organized as a stack, which
 * represents client-focus history. Each client contains a bit array to
 * indicate the tags of a client.
 *
 * To understand everything else, start reading main().
 */



#include <config/config.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <core/dwm.h>
#include <core/monitor.h>
#include <core/statusbar.h>
#include <core/keys.h>
#include <core/client.h>
#include <core/layout.h>
#include <core/tags.h>
#include <core/xevents.h>
#include <xlib/xlib.h>
#include <utils/log.h>
#include <utils/utils.h>


/* local/static prototypes */
static int xevent_hdlr(void);


/* global variables */
dwm_t dwm = {
	.stack = 0x0,
	.layout = __start_layouts,
	.state = DWM_ERROR,
	.numlock_mask = 0,
	.tag_mask = 1,
};


/* global functions */
int dwm_setup(void){
	int r = 0;
	struct sigaction sa;


	if(log_init(CONFIG_LOG_FILE, true) != 0)
		return ERROR("opening log-file %s\n", CONFIG_LOG_FILE);

	DEBUG("dwm hello\n");

	/* do not transform children into zombies when they terminate */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT | SA_RESTART;
	sa.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &sa, 0x0);

	/* clean up any zombies (inherited from .xinitrc etc) immediately */
	while(waitpid(-1, 0x0, WNOHANG) > 0);

	/* init xlib components */
	if(xlib_init() != 0)
		return ERROR("initialising x-server connection\n");

	/* init dwm event handler */
	dwm.event_fd = epoll_create(1);

	if(dwm.event_fd == -1)
		return ERROR("initialising event loop\n");

	if(dwm_hdlr_add(ConnectionNumber(dwm.dpy), xevent_hdlr))
		return ERROR("adding xlib event handler\n");

	/* init core components */
	monitor_discover();
	statusbar_init(CONFIG_STATUSBAR_HEIGHT);
	r |= keys_init();
	r |= clients_init();

	if(r != 0)
		return ERROR("initialising base components\n");

	layout_arrange();
	xlib_sync();

	dwm.state = DWM_RUN;

	return 0;
}

void dwm_cleanup(void){
	layout_t nop = { "", 0x0 };


	tags_set(&dwm.tag_mask, ~0);
	dwm.layout = &nop;

	clients_cleanup();
	keys_cleanup();
	statusbar_destroy();
	monitor_cleanup();

	close(dwm.event_fd);

	xlib_cleanup();
	log_cleanup();
}

void dwm_run(void){
	int n;
	struct epoll_event evts[2];


	while(dwm.state == DWM_RUN){
		// handle xevents that occurred after the last epoll notification
		// and haven't been processed yet
		xevents_handle_events();

		n = epoll_wait(dwm.event_fd, evts, LENGTH(evts), -1);

		for(; n>0; n--){
			if(((event_hdlr_t)evts[n - 1].data.ptr)() != 0)
				return;
		}
	}
}

int dwm_hdlr_add(int fd, event_hdlr_t hdlr){
	struct epoll_event ev;


	ev.events = EPOLLIN;
	ev.data.ptr = hdlr;

	return epoll_ctl(dwm.event_fd, EPOLL_CTL_ADD, fd, &ev);
}


/* local functions */
static int xevent_hdlr(void){
	return xevents_handle_events();
}
