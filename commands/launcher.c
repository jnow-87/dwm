#include <unistd.h>
#include <core/launcher.h>
#include <utils/exec.h>
#include <utils/menu.h>
#include <utils/vector.h>
#include <commands.h>
#include <rc.h>


/* global functions */
void cmd_launcher_menu(cmd_arg_t const *arg){
	int n = 0;
	char const *names[rc.launcher.size];
	launcher_item_t *item;


	vector_for_each(&rc.launcher, item){
		names[n++] = item->name;
	}

	n = menu(names, n);

	if(n == -1)
		return;

	item = vector_get(&rc.launcher, n);
	exec(item->argv.buf);
}
