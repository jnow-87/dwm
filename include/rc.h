#ifndef RC_H
#define RC_H


#include <stdbool.h>
#include <stdio.h>
#include <utils/log.h>
#include <utils/vector.h>


/* types */
typedef enum{
	RC_STATUSBAR_TOP = 0,
	RC_STATUSBAR_BOTTOM,
} rc_statusbar_loc_t;

typedef struct{
	rc_statusbar_loc_t location;

	unsigned int height,
				 padding;

	char *icon_spacer_left,
		 *icon_spacer_intra_left,
		 *icon_spacer_right,
		 *icon_tags_multi,
		 *icon_launcher,
		 *icon_keylock,
		 *icon_zaphod;
} rc_statusbar_t;

typedef struct{
	unsigned int tiled_master_ratio,
				 tiled_master_windows;
} rc_layouts_t;

typedef struct{
	char *font;

	unsigned int border_pixel,
				 snap_pixel,
				 win_resize_pixel,
				 win_move_pixel;

	unsigned int fade_steps,
				 fade_delay_ms;

	bool xinerama,
		 mouse_move;

	rc_statusbar_t statusbar;
	rc_layouts_t layouts;

	char *log_file;
	log_lvl_t log_level;

	vector_t keys,
			 buttons,
			 tags,
			 schemes,
			 launcher;
} rc_t;


/* external variables */
extern rc_t rc;


/* prototypes */
int rc_init(void);
void rc_cleanup(void);

FILE *rc_open(char const **dir, char const **file);

char *rc_stralloc(char *s, size_t n);
void rc_strfree(char *s);
void rc_strlist_free(vector_t *v);


#endif // RC_H
