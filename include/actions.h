#ifndef ACTION_H
#define ACTION_H


/* types */
typedef union{
	int i;
	unsigned int ui;
	float f;
	void const *v;
} action_arg_t;

typedef void (*action_t)(action_arg_t const *);


/* prototypes */
void action_lifecycle(action_arg_t const *arg);

void action_statusbar_toggle(action_arg_t const *arg);
void action_layout_select(action_arg_t const *arg);
void action_spawn(action_arg_t const *arg);
void action_dmenu_run(action_arg_t const *arg);

void action_client_cycle(action_arg_t const *arg);
void action_client_kill(action_arg_t const *arg);
void action_client_move(action_arg_t const *arg);
void action_client_move_mouse(action_arg_t const *arg);
void action_client_resize(action_arg_t const *arg);
void action_client_resize_mouse(action_arg_t const *arg);

void action_tags_view(action_arg_t const *arg);
void action_tags_toggle(action_arg_t const *arg);
void action_tags_client_set(action_arg_t const *arg);
void action_tags_client_toggle(action_arg_t const *arg);


#endif // ACTION_H
