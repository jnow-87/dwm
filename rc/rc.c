#include <config/config.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <core/buttons.h>
#include <core/keys.h>
#include <core/launcher.h>
#include <core/scheme.h>
#include <rc/parser.tab.h>
#include <utils/log.h>
#include <utils/string.h>
#include <utils/vector.h>
#include <rc.h>


/* macros */
#define RC_INITIALISER() (rc_t){ \
	.font = CONFIG_FONT, \
	.border_pixel = CONFIG_BORDER_PIXEL, \
	.snap_pixel = CONFIG_SNAP_PIXEL, \
	.win_resize_pixel = CONFIG_WIN_RESIZE_DELTA, \
	.win_move_pixel = CONFIG_WIN_MOVE_DELTA, \
	.fade_steps = CONFIG_FADE_STEPS, \
	.fade_delay_ms = CONFIG_FADE_DELAY_MS, \
	.xinerama = CONFIG_XINERAMA, \
	.mouse_move = false, \
	.log_level = LOG_ERROR, \
	.log_file = CONFIG_LOG_FILE, \
	.statusbar = { \
		.location = RC_STATUSBAR_TOP, \
		.height = CONFIG_STATUSBAR_HEIGHT, \
		.padding = CONFIG_STATUSBAR_PADDING, \
		.icon_spacer_left = CONFIG_STATUSBAR_SPACER_LEFT, \
		.icon_spacer_intra_left = CONFIG_STATUSBAR_SPACER_INTRA_LEFT, \
		.icon_spacer_right = CONFIG_STATUSBAR_SPACER_RIGHT, \
		.icon_tags_multi = CONFIG_STATUSBAR_TAGS_MULTI, \
		.icon_launcher = CONFIG_STATUSBAR_LAUNCHER_ICON, \
		.icon_keylock = CONFIG_STATUSBAR_KEYLOCK_ICON, \
		.icon_zaphod = CONFIG_STATUSBAR_ZAPHOD_ICON, \
	}, \
	.layouts = { \
		.tiled_master_ratio = CONFIG_TILED_MASTER_RATIO, \
		.tiled_master_windows = CONFIG_TILED_MASTER_WINDOWS, \
	}, \
	.keys = VECTOR_INITIALISER(sizeof(keymap_t)), \
	.buttons = VECTOR_INITIALISER(sizeof(buttonmap_t)), \
	.tags = VECTOR_INITIALISER(sizeof(char*)), \
	.schemes = VECTOR_INITIALISER(sizeof(color_scheme_t)), \
	.launcher = VECTOR_INITIALISER(sizeof(launcher_item_t)), \
};


/* local/static prototypes */
static FILE *open_file(char const *dir, char const *file);

static int verify(void);

static int check_str(char *s, char const *name);
static int check_uint(unsigned int v, unsigned int min, unsigned int max, char const *name);


/* global variables */
rc_t rc = RC_INITIALISER();


/* static variables */
static vector_t rc_strings = VECTOR_INITIALISER(sizeof(char*));


/* global functions */
int rc_init(void){
	rc = RC_INITIALISER();
	rc_strings = VECTOR_INITIALISER(sizeof(char*));

	return -(rcparse() != 0 || verify() != 0);
}

void rc_cleanup(void){
	char **s;
	launcher_item_t *item;


	vector_destroy(&rc.tags);
	vector_destroy(&rc.keys);
	vector_destroy(&rc.buttons);
	vector_destroy(&rc.schemes);

	vector_for_each(&rc.launcher, item)
		launcher_cleanup(item);

	vector_destroy(&rc.launcher);

	vector_for_each(&rc_strings, s)
		free(*s);

	vector_destroy(&rc_strings);
}

FILE *rc_open(char const **dir, char const **file){
	char const *locations[][2] = {
		{ getenv("HOME"), ".dwmrc" },
		{ getenv("HOME"), ".config/dwm/dwmrc" },
		{ getenv("XDG_CONFIG_HOME"), "dwm/dwmrc" },
		{ "/etc", "dwmrc" },
		{ 0x0, 0x0 },
	};
	FILE *fp;


	for(size_t i=0; locations[i][1]!=0x0; i++){
		fp = open_file(locations[i][0], locations[i][1]);

		if(fp != 0x0){
			*dir = locations[i][0];
			*file = locations[i][1];

			return fp;
		}
	}

	return 0x0;
}

char *rc_stralloc(char *s, size_t n){
	char *x;


	x = strnalloc(s, n);

	if(x == 0x0)
		goto err_0;

	if(vector_add(&rc_strings, &x) != 0)
		goto err_1;

	return x;


err_1:
	free(x);

err_0:
	STRERROR("allocating string");

	return 0x0;
}

void rc_strfree(char *s){
	size_t i;
	char **x;


	i = 0;

	vector_for_each(&rc_strings, x){
		if(*x == s)
			break;

		i++;
	}

	if(*x != s){
		ERROR("unable to find string \"%s\"\n", s);
		return;
	}

	vector_rm(&rc_strings, i);
	free(s);
}

void rc_strlist_free(vector_t *v){
	char **s;


	vector_for_each(v, s)
		rc_strfree(*s);

	vector_destroy(v);
}


/* local functions */
static FILE *open_file(char const *dir, char const *file){
	FILE *fp = 0x0;
	int fd,
		dfd;


	if(dir == 0x0 || file == 0x0)
		return 0x0;

	dfd = open(dir, O_RDONLY);

	if(dfd < 0)
		return 0x0;

	fd = openat(dfd, file, O_RDONLY);

	if(fd != -1)
		fp = fdopen(fd, "r");

	close(dfd);

	return fp;
}

static int verify(void){
	int r = 0;


	r |= check_str(rc.font, "font");
	r |= check_uint(rc.fade_steps, 1, UINT_MAX, "winfade steps");

	r |= check_str(rc.log_file, "log-file");

	r |= check_uint(rc.statusbar.height, 1, UINT_MAX, "statusbar height");
	r |= check_str(rc.statusbar.icon_spacer_left, "statusbar left spacer icon");
	r |= check_str(rc.statusbar.icon_spacer_intra_left, "statusbar left-intra spacer icon");
	r |= check_str(rc.statusbar.icon_spacer_right, "statusbar right spacer icon");
	r |= check_str(rc.statusbar.icon_tags_multi, "statusbar multi-tag icon");
	r |= check_str(rc.statusbar.icon_launcher, "statusbar launcher icon");
	r |= check_str(rc.statusbar.icon_keylock, "statusbar keylock icon");
	r |= check_str(rc.statusbar.icon_zaphod, "statusbar zaphod icon");

	r |= check_uint(rc.layouts.tiled_master_ratio, 1, UINT_MAX, "tiled layout master ratio");
	r |= check_uint(rc.layouts.tiled_master_windows, 1, UINT_MAX, "tiled layout master windows");

	r |= schemes_verify();

	return r;
}

static int check_str(char *s, char const *name){
	return (s == 0x0 || *s == 0) ? ERROR("missing config: %s\n", name) : 0;
}

static int check_uint(unsigned int v, unsigned int min, unsigned int max, char const *name){
	return (v < min || v > max) ? ERROR("config out of range: %s has to be between %u and %u\n", name, min, max) : 0;
}
