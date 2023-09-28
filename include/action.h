#ifndef ACTION_H
#define ACTION_H


/* types */
typedef enum{
	ClkTagBar,
	ClkLtSymbol,
	ClkStatusText,
	ClkWinTitle,
	ClkClientWin,
	ClkRootWin,
	ClkLast
} clicks_t;

typedef union{
	int i;
	unsigned int ui;
	float f;
	void const *v;
} action_arg_t;

typedef struct{
	unsigned int click;
	unsigned int mask;
	unsigned int button;
	void (*func)(action_arg_t const *arg);
	action_arg_t const arg;
} button_map_t;

typedef struct{
	unsigned int mod;
	KeySym keysym;
	void (*func)(action_arg_t const *);
	action_arg_t const arg;
} key_map_t;


/* prototypes */
void action_focusstack(action_arg_t const *arg);
void action_killclient(action_arg_t const *arg);
void action_movemouse(action_arg_t const *arg);
void action_moveclient(action_arg_t const *arg);
void action_reszclient(action_arg_t const *arg);
void action_quit(action_arg_t const *arg);
void action_restart(action_arg_t const *arg);
void action_resizemouse(action_arg_t const *arg);
void action_setlayout(action_arg_t const *arg);
void action_spawn(action_arg_t const *arg);
void action_togglebar(action_arg_t const *arg);
void action_tags_view(action_arg_t const *arg);
void action_tags_toggle(action_arg_t const *arg);
void action_client_tags_set(action_arg_t const *arg);
void action_client_tags_toggle(action_arg_t const *arg);


#endif // ACTION_H
