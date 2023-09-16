#ifndef ACTION_H
#define ACTION_H


/* types */
typedef union{
	int i;
	unsigned int ui;
	float f;
	const void *v;
} action_arg_t;


/* prototypes */
void action_focusmon(const action_arg_t *arg);
void action_focusstack(const action_arg_t *arg);
void action_incnmaster(const action_arg_t *arg);
void action_killclient(const action_arg_t *arg);
void action_movemouse(const action_arg_t *arg);
void action_moveclient(const action_arg_t *arg);
void action_reszclient(const action_arg_t *arg);
void action_quit(const action_arg_t *arg);
void action_restart(const action_arg_t *arg);
void action_resizemouse(const action_arg_t *arg);
void action_setlayout(const action_arg_t *arg);
void action_setmfact(const action_arg_t *arg);
void action_spawn(const action_arg_t *arg);
void action_tag(const action_arg_t *arg);
void action_tagmon(const action_arg_t *arg);
void action_togglebar(const action_arg_t *arg);
void action_togglefloating(const action_arg_t *arg);
void action_toggletag(const action_arg_t *arg);
void action_toggleview(const action_arg_t *arg);
void action_view(const action_arg_t *arg);
void action_zoom(const action_arg_t *arg);


#endif // ACTION_H
