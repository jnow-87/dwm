%define api.prefix {rc}
%define parse.error verbose
%define parse.lac full
%locations

/* header */
%{
	#include <stdbool.h>
	#include <stdio.h>
	#include <core/buttons.h>
	#include <core/keys.h>
	#include <core/launcher.h>
	#include <core/scheme.h>
	#include <utils/log.h>
	#include <rc/lexer.lex.h>
	#include <rc.h>


	/* macros */
	// parser error message
	#define EABORT(expr, fmt, ...){ \
		if((expr) != 0){ \
			if(fmt != 0x0 && *fmt != 0) \
				rcerror(fmt, ##__VA_ARGS__); \
			\
			YYERROR; \
		} \
	}

	#define VERIFY(type, val) \
		EABORT(type ##_verify(val) != 0, #type " incomplete")

	#define VECTOR_ADD(vec, item) \
		EABORT(vector_add(vec, item) != 0, strerror(errno));


	/* local/static variables */
	static FILE *rc_fp = 0;
	static char const *rc_dir = 0x0,
					  *rc_file = 0x0;


	/* prototypes */
	void rcunput(char c);


	/* local/static prototypes */
	static int rcerror(char const *s);
	static void cleanup(void);
%}

%code requires{
	#include <core/buttons.h>
	#include <core/keys.h>
	#include <core/launcher.h>
	#include <core/scheme.h>
	#include <rc.h>
}

/* init code */
%initial-action{
	/* open input file */
	rc_fp = rc_open(&rc_dir, &rc_file);

	if(rc_fp == 0x0)
		return ERROR("no rc-file found\n");

	/* start lexer */
	rcrestart(rc_fp);
}

/* parser union type */
%union{
	int i;
	char *s;
	bool b;
	vector_t slist;
	unsigned int modifiers;

	struct{
		cmd_action_t action;
		cmd_arg_t arg;
	} cmd;

	keymap_t key;
	buttonmap_t button;
	color_scheme_t scheme;
	launcher_item_t launcher_item;

	log_lvl_t log_level;
	button_loc_t button_loc;
	scheme_id_t scheme_id;
	rc_statusbar_loc_t statusbar_loc;
}

/* terminals */
// literal tokens
%token BG
%token BORDER
%token BORDER_PIXEL
%token BOTTOM
%token BUTTONS
%token CLIENT
%token CMD
%token DEBUG
%token ERROR
%token FADE_DELAY_MS
%token FADE_STEPS
%token FG
%token FOCUS
%token FONT
%token HEIGHT
%token ICONS
%token INFO
%token KEYLOCK
%token KEYS
%token LAUNCHER
%token LAUNCHER_ENTRIES
%token LAYOUT
%token LAYOUTS
%token LOCATION
%token LOG_FILE
%token LOG_LEVEL
%token MODS
%token MOUSE_MOVE
%token NORMAL
%token NUM
%token PADDING
%token ROOT
%token SCHEMES
%token SNAP_PIXEL
%token SPACER_INTRA
%token SPACER_INTRA_LEFT
%token SPACER_LEFT
%token SPACER_NORM
%token SPACER_RIGHT
%token SPACER_STATUS
%token STATUS
%token STATUSBAR
%token SYM
%token TAGBAR
%token TAGS
%token TAGS_MULTI
%token TILED_MST_RATIO
%token TILED_MST_WIN
%token TOP
%token WIN_MOVE_DT
%token WIN_MOVE_PIXEL
%token WIN_RESIZE_PIXEL
%token XINERAMA
%token ZAPHOD

// value tokens
%token <s> IDFR
%token <i> INT
%token <s> STR
%token <b> BOOL

/* non-terminals */
%type <log_level> log-level
%type <key> key
%type <button> button
%type <button_loc> button-loc
%type <modifiers> modifiers
%type <cmd> command
%type <scheme> scheme
%type <scheme_id> scheme-id
%type <launcher_item> launcher-item
%type <statusbar_loc> statusbar-loc
%type <slist> str-list
%type <slist> str-list-body
%type <s> str


%%


/* start */
start : error												{ cleanup(); rc_cleanup(); YYABORT; }
	  | rc													{ cleanup(); }
	  ;

/* rc */
rc : %empty													{ }
   | rc configs ';'											{ }
   | rc KEYS '=' '{' keys optcom'}' ';'						{ }
   | rc BUTTONS '=' '{' buttons optcom '}' ';'				{ }
   | rc TAGS '=' tags ';'									{ }
   | rc SCHEMES '=' '{' schemes '}' ';'						{ }
   | rc LAUNCHER_ENTRIES '=' '{' launcher-items '}' ';'		{ }
   | rc STATUSBAR '=' '{' statusbar optcom '}' ';' 			{ }
   | rc LAYOUTS '=' '{' layouts optcom '}' ';'				{ }
   ;

configs : FONT '=' str										{ rc.font = $3; }
		| BORDER_PIXEL '=' INT								{ rc.border_pixel = $3; }
		| SNAP_PIXEL '=' INT								{ rc.snap_pixel = $3; }
		| WIN_RESIZE_PIXEL '=' INT							{ rc.win_resize_pixel = $3; }
		| WIN_MOVE_PIXEL '=' INT							{ rc.win_move_pixel = $3; }
		| FADE_STEPS '=' INT								{ rc.fade_steps = $3; }
		| FADE_DELAY_MS '=' INT								{ rc.fade_delay_ms = $3; }
		| XINERAMA '=' BOOL									{ rc.xinerama = $3; }
		| MOUSE_MOVE '=' BOOL								{ rc.mouse_move = $3; }
		| LOG_LEVEL '=' log-level 							{ rc.log_level = $3; }
		| LOG_FILE '=' str									{ rc.log_file = $3; }
		;

log-level : ERROR											{ $$ = LOG_ERROR; }
		  | INFO											{ $$ = LOG_INFO; }
		  | DEBUG											{ $$ = LOG_DEBUG; }
		  ;

keys : %empty												{ rcunput(','); }
	 | keys ',' '{' key optcom '}'							{ VERIFY(key, &$4); VECTOR_ADD(&rc.keys, &$4); }
	 ;

key : %empty												{ $$ = KEY_INITIALISER(); rcunput(','); }
	| key ',' SYM '=' str									{ $$ = $1; $$.keysym = XStringToKeysym($5); rc_strfree($5); }
	| key ',' CMD '=' command								{ $$ = $1; $$.action = $5.action; $$.arg = $5.arg; }
	| key ',' MODS '=' modifiers							{ $$ = $1; $$.mods = $5;  }
	;

buttons : %empty											{ rcunput(','); }
		| buttons ',' '{' button optcom '}'					{ VERIFY(button, &$4); VECTOR_ADD(&rc.buttons, &$4); }
		;

button : %empty												{ $$ = BUTTON_INITIALISER(); rcunput(','); }
	   | button ',' NUM '=' INT								{ $$ = $1; $$.button = $5; }
	   | button ',' CMD '=' command							{ $$ = $1; $$.action = $5.action; $$.arg = $5.arg; }
	   | button ',' MODS '=' modifiers						{ $$ = $1; $$.mods = $5; }
	   | button ',' LOCATION '=' button-loc					{ $$ = $1; $$.loc = $5; }
	   ;

button-loc : ROOT											{ $$ = BLOC_ROOT; }
		   | CLIENT											{ $$ = BLOC_CLIENT; }
		   | LAUNCHER										{ $$ = BLOC_LAUNCHER; }
		   | TAGBAR											{ $$ = BLOC_TAGBAR; }
		   | LAYOUT											{ $$ = BLOC_LAYOUT; }
		   ;

modifiers : str-list										{ EABORT(input_modkeys(&$1, &$$) != 0, "parsing key modifiers"); rc_strlist_free(&$1); }
		  ;

command : str												{ EABORT(cmd_parse($1, &$$.action, &$$.arg) != 0, "parsing command"); rc_strfree($1); }
		;

tags : str-list												{ vector_destroy(&rc.tags); vector_mv(&rc.tags, &$1); }
	 ;

schemes : %empty											{ }
		| schemes scheme-id '=' '{' scheme optcom '}' ';'	{ $5.id = $2; VERIFY(scheme, &$5); VECTOR_ADD(&rc.schemes, &$5); }
		;

scheme : %empty												{ $$ = SCHEME_INITIALISER(); rcunput(','); }
	   | scheme ',' FG '=' str								{ $$.fg = $5; }
	   | scheme ',' BG '=' str								{ $$.bg = $5; }
	   | scheme ',' BORDER '=' str							{ $$.border = $5; }
	   ;

scheme-id : NORMAL											{ $$ = SCM_NORM; }
		  | FOCUS											{ $$ = SCM_FOCUS; }
		  | STATUS											{ $$ = SCM_STATUS; }
		  | SPACER_NORM										{ $$ = SCM_SPACER_NORM; }
		  | SPACER_INTRA									{ $$ = SCM_SPACER_INTRA; }
		  | SPACER_STATUS									{ $$ = SCM_SPACER_STATUS; }
		  ;

launcher-items : %empty										{ }
			   | launcher-items launcher-item ';'			{ VECTOR_ADD(&rc.launcher, &$2); }
			   ;

launcher-item : IDFR '=' str								{ EABORT(launcher_init(&$$, $1, $3) != 0, "parsing launcher item"); rc_strfree($3); }
			  ;

layouts : %empty											{ rcunput(','); }
		| layouts ',' TILED_MST_RATIO '=' INT				{ rc.layouts.tiled_master_ratio = $5; }
		| layouts ',' TILED_MST_WIN '=' INT					{ rc.layouts.tiled_master_windows = $5; }
		;

statusbar : %empty											{ rcunput(','); }
		  | statusbar ',' LOCATION '=' statusbar-loc		{ rc.statusbar.location = $5; }
		  | statusbar ',' HEIGHT '=' INT					{ rc.statusbar.height = $5; }
		  | statusbar ',' PADDING '=' INT					{ rc.statusbar.padding = $5; }
		  | statusbar ',' SPACER_LEFT '=' str				{ rc.statusbar.icon_spacer_left = $5; }
		  | statusbar ',' SPACER_INTRA_LEFT '=' str			{ rc.statusbar.icon_spacer_intra_left = $5; }
		  | statusbar ',' SPACER_RIGHT '=' str				{ rc.statusbar.icon_spacer_right = $5; }
		  | statusbar ',' TAGS_MULTI '=' str				{ rc.statusbar.icon_tags_multi = $5; }
		  | statusbar ',' LAUNCHER '=' str					{ rc.statusbar.icon_launcher = $5; }
		  | statusbar ',' KEYLOCK '=' str					{ rc.statusbar.icon_keylock = $5; }
		  | statusbar ',' ZAPHOD '=' str					{ rc.statusbar.icon_zaphod = $5; }
		  ;

statusbar-loc : TOP											{ $$ = RC_STATUSBAR_TOP; }
			  | BOTTOM										{ $$ = RC_STATUSBAR_BOTTOM; }
			  ;


str-list : '[' str-list-body optcom ']'						{ $$ = $2; }
		 ;

str-list-body : %empty										{ $$ = VECTOR_INITIALISER(sizeof(char*)); rcunput(','); }
			  | str-list-body ',' str						{ $$ = $1; VECTOR_ADD(&$$, &$3); }
			  ;
str : STR													{ $$ = $1; EABORT($$ == 0x0, strerror(errno)); };

optcom : %empty												{ }
	   | ','												{ };


%%


/* local functions */
static int rcerror(char const *s){
	fprintf(stderr, "%s/%s:%d:%d token \"%s\" -- %s\n",
		rc_dir,
		rc_file,
		rclloc.first_line,
		rclloc.first_column,
		rctext,
		s
	);

	return -1;
}

static void cleanup(void){
	rclex_destroy();
	fclose(rc_fp);
}
