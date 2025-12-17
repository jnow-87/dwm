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
	void const *v;
} cmd_arg_t;

typedef void (*cmd_t)(cmd_arg_t const *);


/* prototypes */
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


#endif // COMMANDS_H
