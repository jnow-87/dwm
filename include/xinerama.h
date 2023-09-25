#ifndef XINERAMA_H
#define XINERAMA_H


#include <config/config.h>


/* macros */
#ifndef CONFIG_XINERAMA
# define xinerama_discover_monitor()	-1
#endif // CONFIG_XINERAMA


/* prototypes */
#ifdef CONFIG_XINERAMA
int xinerama_discover_monitor(void);
#endif // CONFIG_XINERAMA


#endif // XINERAMA_H
