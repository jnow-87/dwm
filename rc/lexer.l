%option prefix="rc"
%option noyywrap
%option noinput

%{
	#include <rc/parser.tab.h>
	#include <rc.h>


	/* define YY_USER_ACTION, executed for every token, before executing its
	 * action code
	 */
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		rclloc.first_line = rclloc.last_line = rc_line; \
		rclloc.first_column = rc_col; \
		rclloc.last_column = rc_col + rcleng; \
		rc_col += rcleng; \
	}

	/* make yyunput available outside of the lexer */
	static void yyunput(int c, char *buf_ptr);

	void rcunput(char c){
		yyunput(c, (yytext_ptr));
	}
%}

/* start states */
%x MULTI_LINE_COMMENT


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int rc_col = 1;
	static unsigned int rc_line = 1;
%}

	/* literal tokens */
"bg"							{ return BG; }
"border"						{ return BORDER; }
"border-pixel"					{ return BORDER_PIXEL; }
"bottom"						{ return BOTTOM; }
"buttons"						{ return BUTTONS; }
"client"						{ return CLIENT; }
"cmd"							{ return CMD; }
"debug"							{ return DEBUG; }
"error"							{ return ERROR; }
"fade-delay-ms"					{ return FADE_DELAY_MS; }
"fade-steps"					{ return FADE_STEPS; }
"fg"							{ return FG; }
"focus"							{ return FOCUS; }
"font"							{ return FONT; }
"height"						{ return HEIGHT; }
"icons"							{ return ICONS; }
"info"							{ return INFO; }
"keylock"						{ return KEYLOCK; }
"keys"							{ return KEYS; }
"launcher"						{ return LAUNCHER; }
"launcher-entries"				{ return LAUNCHER_ENTRIES; }
"layout"						{ return LAYOUT; }
"layouts"						{ return LAYOUTS; }
"location"						{ return LOCATION; }
"log-file"						{ return LOG_FILE; }
"log-level"						{ return LOG_LEVEL; }
"mods"							{ return MODS; }
"mouse-move"					{ return MOUSE_MOVE; }
"normal"						{ return NORMAL; }
"num"							{ return NUM; }
"padding"						{ return PADDING; }
"root"							{ return ROOT; }
"schemes"						{ return SCHEMES; }
"snap-pixel"					{ return SNAP_PIXEL; }
"spacer-intra"					{ return SPACER_INTRA; }
"spacer-intra-left"				{ return SPACER_INTRA_LEFT; }
"spacer-left"					{ return SPACER_LEFT; }
"spacer-norm"					{ return SPACER_NORM; }
"spacer-right"					{ return SPACER_RIGHT; }
"spacer-status"					{ return SPACER_STATUS; }
"status"						{ return STATUS; }
"statusbar"						{ return STATUSBAR; }
"sym"							{ return SYM; }
"tagbar"						{ return TAGBAR; }
"tags"							{ return TAGS; }
"tags-multi"					{ return TAGS_MULTI; }
"tiled-master-ratio"			{ return TILED_MST_RATIO; }
"tiled-master-windows"			{ return TILED_MST_WIN; }
"top"							{ return TOP; }
"win-move-pixel"				{ return WIN_MOVE_PIXEL; }
"win-resize-pixel"				{ return WIN_RESIZE_PIXEL; }
"xinerama"						{ return XINERAMA; }
"zaphod"						{ return ZAPHOD; }

	/* comments */
\/\/.*\r?\n						{ rc_col = 1; ++rc_line; }
\/\*							{ BEGIN MULTI_LINE_COMMENT; }
<MULTI_LINE_COMMENT>\*\/		{ BEGIN INITIAL; }
<MULTI_LINE_COMMENT>.			{ }
<MULTI_LINE_COMMENT>\r?\n		{ rc_col = 1; ++rc_line; }

	/* bool */
"true"							{ rclval.b = true; return BOOL; }
"false"							{ rclval.b = false; return BOOL; }

	/* identifier */
[a-zA-Z][a-zA-Z_\-0-9]+			{ rclval.s = rc_stralloc(rctext, rcleng); return IDFR; }

	/* string */
\"[^"]*\"						{ rclval.s = rc_stralloc(rctext + 1, rcleng - 2);  return STR; }

	/* integer */
(0x)?[0-9a-fA-F]+				{ rclval.i = strtol(rctext, 0x0, 0); return INT; }

	/* spaces and line breaks */
\r?\n							{ rc_col = 1; ++rc_line; }
[ \t]							{ }

	/* default */
.								{ return rctext[0]; }
