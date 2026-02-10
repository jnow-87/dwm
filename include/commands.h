#ifndef COMMANDS_H
#define COMMANDS_H


#include <stdarg.h>


/* macros */
#define SPAWN(...) (char const *[]){ __VA_ARGS__, 0x0 }


/* types */
typedef union{
	int i;
	unsigned int ui;
	float f;
	void *v;
} cmd_arg_t;

typedef enum{
	ARG_INVAL = -1,
	ARG_FORWARD = 1,
	ARG_BACKWARD,
	ARG_ZAPHOD,
	ARG_IGNORE_ZAPHOD,
	ARG_ALT,
	ARG_CTRL,
	ARG_SHIFT,
	ARG_WIN,
	ARG_LEFT,
	ARG_RIGHT,
	ARG_UP,
	ARG_DOWN,
	ARG_TOP,
	ARG_BOTTOM,
	ARG_BORDER,
	ARG_SHRINK,
	ARG_GROW,
	ARG_VERT,
	ARG_HOR,
} arg_id_t;

typedef void (*cmd_action_t)(cmd_arg_t const *);
typedef int (*cmd_argparse_t)(char const *tk, arg_id_t tk_id, cmd_arg_t *args);

typedef struct cmd_t{
	char const *name;

	cmd_action_t action;
	cmd_argparse_t parse;
} cmd_t;

typedef struct arg_keyword_t{
	char const *keyword;
	arg_id_t id;
} arg_keyword_t;


/* prototypes */
int cmd_parse(char *cmdline, cmd_action_t *action, cmd_arg_t *args);
arg_id_t cmd_keyword_parse(char const *tk);

// command exec functions
void cmd_lifecycle(cmd_arg_t const *arg);

void cmd_statusbar_toggle(cmd_arg_t const *arg);
void cmd_layout_select(cmd_arg_t const *arg);
void cmd_spawn(cmd_arg_t const *arg);
void cmd_dmenu_run(cmd_arg_t const *arg);
void cmd_launcher_menu(cmd_arg_t const *arg);

void cmd_client_cycle(cmd_arg_t const *arg);
void cmd_client_kill(cmd_arg_t const *arg);
void cmd_client_move(cmd_arg_t const *arg);
void cmd_client_move_mouse(cmd_arg_t const *arg);
void cmd_client_resize(cmd_arg_t const *arg);
void cmd_client_resize_mouse(cmd_arg_t const *arg);
void cmd_client_max(cmd_arg_t const *arg);
void cmd_client_fullscreen(cmd_arg_t const *arg);

void cmd_tags_view(cmd_arg_t const *arg);
void cmd_tags_toggle(cmd_arg_t const *arg);
void cmd_tags_client_set(cmd_arg_t const *arg);
void cmd_tags_client_toggle(cmd_arg_t const *arg);
void cmd_tags_menu(cmd_arg_t const *arg);

void cmd_winfade_add(cmd_arg_t const *arg);
void cmd_winfade_fade(cmd_arg_t const *arg);

void cmd_keylock_set(cmd_arg_t const *arg);
void cmd_keylock_toggle(cmd_arg_t const *arg);

void cmd_zaphod_toggle(cmd_arg_t const *arg);

// command argument parsers
int cmd_spawn_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);
int cmd_client_cycle_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);
int cmd_client_move_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);
int cmd_client_resize_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);
int cmd_client_max_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);
int cmd_tag_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);
int cmd_winfade_parse(char const *tk, arg_id_t tk_id, cmd_arg_t *arg);


#endif // COMMANDS_H
