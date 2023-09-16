/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H


#include <action.h>
#include <layout.h>
#include <rule.h>


/* macros */
#define TAGMASK                 ((1 << ntags) - 1)


// TODO replace the size variables n*, e.g. by using linker arrays
extern const char *tags[];
extern unsigned int ntags;
extern const rule_t rules[];
extern unsigned int nrules;
extern const key_map_t keys[];
extern const unsigned int nkeys;
extern const button_map_t buttons[];
extern const unsigned int nbuttons;
extern const layout_t layouts[];
extern unsigned int nlayouts;
extern const char *colors[][3];
extern unsigned int ncolors;

#endif // CONFIG_H
