/* See LICENSE file for copyright and license details. */

#ifndef CONFIG_H
#define CONFIG_H


#include <stddef.h>
#include <core/layout.h>
#include <xlib/gfx.h>
#include <xlib/input.h>


/* external variables */
// TODO replace the size variables n*, e.g. by using linker arrays
extern char const *tags[];
extern key_map_t const keys[];
extern button_map_t const buttons[];
extern layout_t const layouts[];
extern char const *colors[NSCMS][3];

extern size_t nlayouts,
			  nkeys,
			  nbuttons,
			  ntags;


#endif // CONFIG_H
