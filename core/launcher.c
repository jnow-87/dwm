#include <stdlib.h>
#include <string.h>
#include <core/launcher.h>
#include <utils/log.h>
#include <utils/string.h>
#include <utils/vector.h>


/* global functions */
int launcher_init(launcher_item_t *item, char const *name, char *cmdline){
	char *tk,
		 *s;


	item->name = name;

	if(vector_init(&item->argv, sizeof(char*), 2) != 0)
		goto err_0;

	tk = strtok(cmdline, " ");

	while(tk != 0x0){
		s = stralloc(tk);
		tk = strtok(0x0, " ");

		if(s == 0x0 || vector_add(&item->argv, &s) != 0)
			goto err_1;
	}

	if(vector_add(&item->argv, &((char*){ 0x0 })) != 0)
		goto err_1;

	if(item->argv.size < 2)
		goto err_1;

	return 0;


err_1:
	launcher_cleanup(item);
	free(s);

err_0:
	return ERROR("allocating launcher item\n");
}

void launcher_cleanup(launcher_item_t *item){
	char **s;


	vector_for_each(&item->argv, s)
		free(*s);

	vector_destroy(&item->argv);
}
