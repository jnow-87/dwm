#ifndef STATUSBAR_H
#define STATUSBAR_H


#include <dwm.h>


/* external variables */
extern char stext[];


/* prototypes */
void drawbar(monitor_t *m);
void drawbars(void);
void updatebars(void);
void updatebarpos(monitor_t *m);
void updatestatus(void);
void updatetitle(client_t *c);


#endif // STATUSBAR_H
