#ifndef XINERAMA_H
#define XINERAMA_H


#include <config/config.h>


/* macros */
#ifndef CONFIG_XINERAMA
# define xinerama_discover()	-1
#endif // CONFIG_XINERAMA


/* prototypes */
#ifdef CONFIG_XINERAMA
int xinerama_discover(void);
#endif // CONFIG_XINERAMA


#endif // XINERAMA_H
