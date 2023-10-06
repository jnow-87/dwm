#ifndef INPUT_H
#define INPUT_H


/* prototypes */
int input_get_root_pointer(int *x, int *y);
void input_grab_keys(void);
void input_grab_buttons(client_t *c, int focused);


#endif // INPUT_H
