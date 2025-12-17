#include <unistd.h>
#include <core/launcher.h>
#include <utils/menu.h>
#include <commands.h>


/* global functions */
void cmd_launcher_menu(cmd_arg_t const *arg){
	int n = 0;
	char const *names[__stop_launcher_items - __start_launcher_items];
	launcher_item_t *item;


	config_for_each(launcher_items, item){
		names[n++] = item->name;
	}

	n = menu(names, n);

	if(n == -1)
		return;

	if(fork() == 0)
		execl("/bin/sh", "sh", "-c", __start_launcher_items[n].cmd, 0x0);
}
