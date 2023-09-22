/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H


#include <action.h>
#include <layout.h>
#include <rule.h>


/* macros */
#define TAGMASK ((1 << ntags) - 1)


// TODO replace the size variables n*, e.g. by using linker arrays
extern char const *tags[];
extern unsigned int ntags;
extern rule_t const rules[];
extern unsigned int nrules;
extern key_map_t const keys[];
extern unsigned int const nkeys;
extern button_map_t const buttons[];
extern unsigned int const nbuttons;
extern layout_t const layouts[];
extern unsigned int nlayouts;
extern char const *colors[][3];
extern unsigned int ncolors;


#endif // CONFIG_H
