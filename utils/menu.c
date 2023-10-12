#include <config/config.h>
#include <core/dwm.h>
#include <core/statusbar.h>
#include <xlib/gfx.h>
#include <xlib/input.h>
#include <xlib/window.h>
#include <xlib/xlib.h>
#include <utils/utils.h>


/* macros */
#define PADDING			CONFIG_STATUSBAR_PADDING
#define ENTRY_HEIGHT	CONFIG_STATUSBAR_HEIGHT
#define TEXTW(s)		(gfx_text_width(dwm.gfx, s) + PADDING)

#define WITHIN(_x, _y, _geom)	((_x) >= 0 && (_x) < (_geom).width && (_y) >= 0 && (_y) < (_geom).height)


/* types */
typedef struct{
	window_t win;
	win_geom_t geom;

	char const **entries;
	int nentries;
} menu_t;


/* local/static prototypes */
static int init(char const **entries, int n, menu_t *menu);
static void destroy(menu_t *menu);
static void render(menu_t *menu, int selected);


/* global functions */
int menu(char const **entries, int n){
	int selected[2] = { 0 };
	Time tlast = 0;
	menu_t menu;
	xevent_t ev;
	keysym_t key;


	/* init */
	if(init(entries, n, &menu) != 0)
		return -1;

	if(input_pointer_grab(menu.win, dwm.gfx->cursors[CUR_NORM]) != 0)
		return -1;

	render(&menu, selected[0]);

	/* event loop */
	while(xlib_get_event(&ev, true, PointerMotionMask | ButtonPressMask | KeyPressMask) >= 0){
		// key events
		if(ev.type == KeyPress){
			key = input_keysym(ev.xkey.keycode);

			if(key == XK_Escape)
				selected[0] = -1;

			if(key == XK_Escape || key == XK_Return)
				break;

			switch(key){
			case XK_Up:		selected[0] = MAX(0, selected[0] - 1); break;
			case XK_Tab:	// fall through
			case XK_Down:	selected[0] = MIN(menu.nentries - 1, selected[0] + 1); break;
			}
		}
		// pointer movement
		else if(ev.type == MotionNotify){
			if((ev.xmotion.time - tlast) <= (1000 / 60))
				continue;

			tlast = ev.xmotion.time;

			if(WITHIN(ev.xmotion.x, ev.xmotion.y, menu.geom)){
				selected[0] = ev.xmotion.y / ENTRY_HEIGHT;
			}
		}
		// click
		else if(ev.type == ButtonPress && ev.xbutton.button == 1)
			break;

		// render if updated
		if(selected[0] != selected[1]){
			render(&menu, selected[0]);
			selected[1] = selected[0];
		}
	}

	/* cleanup */
	input_pointer_release();
	destroy(&menu);

	if(ev.type == ButtonPress && !WITHIN(ev.xbutton.x, ev.xbutton.y, menu.geom))
		return -1;

	return selected[0];
}


/* local functions */
static int init(char const **entries, int n, menu_t *menu){
	win_geom_t *geom = &menu->geom;
	kbd_map_t map;


	if(n <= 0)
		return -1;

	geom->height = n * ENTRY_HEIGHT;
	geom->width = 0;

	for(int i=0; i<n; i++)
		geom->width = MAX(geom->width, TEXTW(entries[i]));

	if(geom->width <= 0)
		return -1;

	if(input_pointer_coord(&geom->x, &geom->y) != 0)
		return -1;

	if(input_kbd_map_init(&map) != 0)
		return -1;

	menu->win = win_create(geom, CUR_NORM, "dwm-menu", true);
	menu->entries = entries;
	menu->nentries = n;

	input_key_register(menu->win, XK_Escape, 0, &map);
	input_key_register(menu->win, XK_Return, 0, &map);
	input_key_register(menu->win, XK_Tab, 0, &map);
	input_key_register(menu->win, XK_Up, 0, &map);
	input_key_register(menu->win, XK_Down, 0, &map);
	input_kbd_map_release(&map);

	win_focus(menu->win);
	input_pointer_move(menu->win, geom->width / 2, ENTRY_HEIGHT / 2);

	return 0;
}

static void destroy(menu_t *menu){
	win_destroy(menu->win);

	if(dwm.focused != 0x0)
		win_focus(dwm.focused->win);

	statusbar_raise();
}

static void render(menu_t *menu, int selected){
	for(size_t i=0; i<menu->nentries; i++)
		gfx_text(dwm.gfx, 0, i * ENTRY_HEIGHT, menu->geom.width, ENTRY_HEIGHT, (i == selected) ? SCM_FOCUS : SCM_NORM, PADDING / 2, menu->entries[i], 0);

	gfx_map(dwm.gfx, menu->win, 0, 0, menu->geom.width, menu->geom.height);
}
