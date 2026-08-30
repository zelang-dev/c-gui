#ifndef _GUI_H
#define _GUI_H

typedef enum {
	field_text = 4900,
	field_number,
	field_date,
	field_email,
	field_secret,
	field_url,
	field_regex,
} ui_field_type;

enum {
	ID_GUI_ICON = 900,
	ID_GUI_MENU = 1000,
	ID_GUI_CONTROL = 2000,
	ID_GUI_CONFIRM,
	ID_GUI_CANCEL,
	ID_GUI_STATUS = 3000,
	ID_GUI_ERROR,
	ID_GUI_VERIFIED,
	ID_GUI_STATIC = 4000,
};

typedef struct Buttons_s {
	char *label;
	int result;
	int ID;
} Button;

#if defined(__APPLE__)
#define USE_COCOA 1
#undef in
#include "Apple/Cocoa.h"
#define in ,
#define lucida			"LucidaGrande"
#define __GUI_MENU__ 	id self, SEL selector, id data
#define __GUI_FILE__ 	id self, NSString file
#define __GUI_FIELD__ 	__GUI_MENU__
#define __GUI_WEBVIEW__	__GUI_MENU__
#elif defined(_WIN32)
#define COBJMACROS
#include <windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <exdisp.h>
#include <mshtmhst.h>
#include <mshtml.h>
#include <shobjidl.h>
#include <WebView2.h>

typedef struct webview2_struct webview2;
struct webview_priv {
	HWND hwnd;
	IOleObject **browser;
	BOOL is_fullscreen;
	DWORD saved_style;
	DWORD saved_ex_style;
	RECT saved_rect;
	webview2 *webview2;
	struct ICoreWebView2Vtbl view;
};

typedef struct _MSGBOXDATA {
	MSGBOXPARAMSW;
	HWND    pwndOwner;          // Internal use only
	DWORD   dwPadding;          // Note: Windows XP has no this field
	WORD    wLanguageId;
	INT *pidButton;          	// Array of button IDs
	LPWSTR *ppszButtonText;     // Array of button text strings
	DWORD   cButtons;           // Number of buttons
	UINT    DefButton;          // Default button ID
	UINT    CancelId;           // Button ID corresponding to Cancel action
	DWORD   dwTimeout;          // Message box timeout
	HWND *phwndList;          	// Internal use only
	DWORD   dwReserved[20];     // Reserved for future use
} MSGBOXDATA, *PMSGBOXDATA;

typedef struct ButtonW {
	PCWSTR label;
	int result;
	int ID;
} ButtonW;

#define main(...)                       \
    main(int argc, char** argv) {       \
		(void)argc;						\
		(void)argv;						\
		return WinMain(0, 0, 0, 0);		\
    }                                   \
    int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmdline, int show)

#define lucida "Lucida Sans"
#define __GUI_MENU__ 	ui_t *self, void *data
#define __GUI_FILE__ 	ui_t *self, const char *file
#define __GUI_FIELD__ 	__GUI_MENU__
#define __GUI_WEBVIEW__	ICoreWebView2 *self, void *data
#else
#define lucida "lucidasans-bold-8"
#define __GUI_MENU__ 	ui_t *self, void *data
#define __GUI_FILE__ 	Widget self, XtPointer client, XtPointer data
#define __GUI_FIELD__ 	__GUI_FILE__
#define __GUI_WEBVIEW__ __GUI_FILE__
#define _DEFAULT_SOURCE 1
#define ARROW_SCROLLBAR 1
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <X11/Xresource.h>
#include <X11/Xos.h>
#include <X11/Intrinsic.h>
#include <X11/Core.h>
#include <X11/Object.h>
#include <X11/Shell.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Xmu.h>
#include <X11/Xmu/Converters.h>
#include <Linux/Mowitz.h>

#ifdef NATIVE_XAW
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/AsciiText.h>

#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Repeater.h>
#else
#include <Linux/Xaw95/Box.h>
#include <Linux/Xaw95/Paned.h>
#include <Linux/Xaw95/Dialog.h>
#include <Linux/Xaw95/Command.h>
#include <Linux/Xaw95/Form.h>
#include <Linux/Xaw95/AsciiText.h>

#include <Linux/Xaw95/MenuButton.h>
#include <Linux/Xaw95/Label.h>
#include <Linux/Xaw95/Viewport.h>
#include <Linux/Xaw95/List.h>
#include <Linux/Xaw95/Scrollbar.h>
#include <Linux/Xaw95/SimpleMenu.h>
#include <Linux/Xaw95/SmeBSB.h>
#include <Linux/Xaw95/SmeLine.h>
#include <Linux/Xaw95/Repeater.h>
#endif

#include <Linux/TextField.h>
#include <Linux/FileSelect.h>
#include <Linux/Gridbox.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

struct webview_priv {
	Widget *window;
	Widget *scroller;
	Widget *webview;
	Widget *inspector_window;
	void **queue;
	int ready;
	int js_busy;
	int should_exit;
};
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#ifndef C_API
#	define C_API extern
#endif

#ifndef FORCEINLINE
#	if defined(_MSC_VER) && !defined(__clang__)
#		define FORCEINLINE __forceinline
#	elif defined(__GNUC__)
#		if defined(__STRICT_ANSI__)
#			define FORCEINLINE __inline__ __attribute__((always_inline))
#		else
#			define FORCEINLINE inline __attribute__((always_inline))
#		endif
#	elif defined(__BORLANDC__) || defined(__DMC__) || defined(__SC__)	\
		|| defined(__WATCOMC__) || defined(__LCC__) || defined(__DECC)
#		define FORCEINLINE __inline
#	else /* No inline support. */
#		define FORCEINLINE
#	endif
#endif

#define RGB_COLOR(r, g, b)		(r),(g),(b)
#define RGB_BLACK				0,0,0
#define RGB_WHITE				255,255,255
#define RGB_RED					255,0,0
#define RGB_LIME				0,255,0
#define RGB_BLUE				0,0,255
#define RGB_YELLOW				255,255,0
#define RGB_CYAN				0,255,255
#define RGB_MAGENTA				255,0,255
#define RGB_SILVER				192,192,192
#define RGB_GRAY				128,128,128
#define RGB_MAROON				128,0,0
#define RGB_OLIVE				128,128,0
#define RGB_GREEN				0,128,0
#define RGB_PURPLE				128,0,128
#define RGB_TEAL				0,128,128
#define RGB_NAVY				0,0,128
#define RGB_MAROON				128,0,0
#define RGB_DARK_RED			139,0,0
#define RGB_BROWN				165,42,42
#define RGB_FIREBRICK			178,34,34
#define RGB_CRIMSON				220,20,60
#define RGB_RED					255,0,0
#define RGB_TOMATO				255,99,71
#define RGB_CORAL				255,127,80
#define RGB_INDIAN_RED			205,92,92
#define RGB_LIGHT_CORAL			240,128,128
#define RGB_DARK SALMON			233,150,122
#define RGB_SALMON				250,128,114
#define RGB_LIGHT_SALMON		255,160,122
#define RGB_ORANGE_RED			255,69,0
#define RGB_DARK_ORANGE			255,140,0
#define RGB_ORANGE				255,165,0
#define RGB_GOLD				255,215,0
#define RGB_DARK_GOLDEN_ROD		184,134,11
#define RGB_GOLDEN_ROD			218,165,32
#define RGB_PALE_GOLDEN_ROD		238,232,170
#define RGB_DARK_KHAKI			189,183,107
#define RGB_KHAK				240,230,140
#define RGB_OLIVE				128,128,0
#define RGB_YELLOW				255,255,0
#define RGB_YELLOW_GREEN		154,205,50
#define RGB_DARK_OLIVE_GREEN	85,107,47
#define RGB_OLIVE_DRAB			107,142,35
#define RGB_LAWN_GREEN			124,252,0
#define RGB_CHARTREUSE			127,255,0
#define RGB_GREEN_YELLOW		173,255,47
#define RGB_DARK_GREEN			0,100,0
#define RGB_GREEN				0,128,0
#define RGB_FOREST_GREEN		34,139,34
#define RGB_LIME				0,255,0
#define RGB_LIME_GREEN			50,205,50
#define RGB_LIGHT_GREEN			144,238,144
#define RGB_PALE_GREEN			152,251,152
#define RGB_DARK_SEA_GREEN		143,188,143
#define RGB_MEDIUM_SPRING_GREEN	0,250,154
#define RGB_SPRING_GREEN		0,255,127
#define RGB_SEA_GREEN			46,139,87
#define RGB_MEDIUM_AQUA_MARINE	102,205,170
#define RGB_MEDIUM_SEA_GREEN	60,179,113
#define RGB_LIGHT_SEA_GREEN		32,178,170
#define RGB_DARK_SLATE_GRAY		47,79,79
#define RGB_TEAL				0,128,128
#define RGB_DARK_CYAN			0,139,139
#define RGB_AQUA				0,255,255
#define RGB_CYAN				0,255,255
#define RGB_LIGHT_CYAN			224,255,255
#define RGB_DARK_TURQUOISE		0,206,209
#define RGB_TURQUOISE			64,224,208
#define RGB_MEDIUM_TURQUOISE	72,209,204
#define RGB_PALE_TURQUOISE		175,238,238
#define RGB_AQUA_MARINE			127,255,212
#define RGB_POWDER_BLUE			176,224,230
#define RGB_CADET_BLUE			95,158,160
#define RGB_STEEL_BLUE			70,130,180
#define RGB_CORN_FLOWER_BLUE	100,149,237
#define RGB_DEEP_SKY_BLUE		0,191,255
#define RGB_DODGER_BLUE			30,144,255
#define RGB_LIGHT_BLUE			173,216,230
#define RGB_SKY_BLUE			135,206,235
#define RGB_LIGHT_SKY_BLUE		135,206,250
#define RGB_MIDNIGHT_BLUE		25,25,112
#define RGB_NAVY				0,0,128
#define RGB_DARK_BLUE			0,0,139
#define RGB_MEDIUM_BLUE			0,0,205
#define RGB_BLUE				0,0,255
#define RGB_ROYAL_BLUE			65,105,225
#define RGB_BLUE_VIOLET			138,43,226
#define RGB_INDIGO				75,0,130
#define RGB_DARK_SLATE_BLUE		72,61,139
#define RGB_SLATE_BLUE			106,90,205
#define RGB_MEDIUM_SLATE_BLUE	123,104,238
#define RGB_MEDIUM_PURPLE		147,112,219
#define RGB_DARK_MAGENTA		139,0,139
#define RGB_DARK_VIOLET			148,0,211
#define RGB_DARK_ORCHID			153,50,204
#define RGB_MEDIUM_ORCHID		186,85,211
#define RGB_PURPLE				128,0,128
#define RGB_THISTLE				216,191,216
#define RGB_PLUM				221,160,221
#define RGB_VIOLET				238,130,238
#define RGB_FUCHSIA				255,0,255
#define RGB_ORCHID				218,112,214
#define RGB_MEDIUM_VIOLET_RED	199,21,133
#define RGB_PALE_VIOLET_RED		219,112,147
#define RGB_DEEP_PINK			255,20,147
#define RGB_HOT_PINK			255,105,180
#define RGB_LIGHT_PINK			255,182,193
#define RGB_PINK				255,192,203
#define RGB_ANTIQUE_WHITE		250,235,215
#define RGB_BEIGE				245,245,220
#define RGB_BISQUE				255,228,196
#define RGB_BLANCHED_ALMOND		255,235,205
#define RGB_WHEAT				245,222,179
#define RGB_CORN_SILK			255,248,220
#define RGB_LEMON_CHIFFON		255,250,205
#define RGB_LIGHT_GOLDEN_ROD_YELLOW	250,250,210
#define RGB_LIGHT_YELLOW		255,255,224
#define RGB_SADDLE_BROWN		139,69,19
#define RGB_SIENNA				160,82,45
#define RGB_CHOCOLATE			210,105,30
#define RGB_PERU				205,133,63
#define RGB_SANDY_BROWN			244,164,96
#define RGB_BURLY_WOOD			222,184,135
#define RGB_TAN					210,180,140
#define RGB_ROSY_BROWN			188,143,143
#define RGB_MOCCASIN			255,228,181
#define RGB_NAVAJO_WHITE		255,222,173
#define RGB_PEACH_PUFF			255,218,185
#define RGB_MISTY_ROSE			255,228,225
#define RGB_LAVENDER_BLUSH		255,240,245
#define RGB_LINEN				250,240,230
#define RGB_OLD_LACE			253,245,230
#define RGB_PAPAYA_WHIP			255,239,213
#define RGB_SEA_SHELL			255,245,238
#define RGB_MINT_CREAM			245,255,250
#define RGB_SLATE_GRAY			112,128,144
#define RGB_LIGHT_SLATE_GRAY	119,136,153
#define RGB_LIGHT_STEEL_BLUE	176,196,222
#define RGB_LAVENDER			230,230,250
#define RGB_FLORAL_WHITE		255,250,240
#define RGB_ALICE_BLUE			240,248,255
#define RGB_GHOST_WHITE			248,248,255
#define RGB_HONEYDEW			240,255,240
#define RGB_IVORY				255,255,240
#define RGB_AZURE				240,255,255
#define RGB_SNOW				255,250,250
#define RGB_BLACK				0,0,0
#define RGB_DIM_GRAY			105,105,105
#define RGB_GREY				128,128,128
#define RGB_DARK_GREY			169,169,169
#define RGB_SILVER				192,192,192
#define RGB_LIGHT_GREY			211,211,211
#define RGB_GAINSBORO			220,220,220
#define RGB_WHITE_SMOKE			245,245,245
#define RGB_WHITE				255,255,255

#define GetNumMenus(X) 		X->num_menus
#define item_name_limit 	50
#define none_selected 		-1
#define no_menu 			-1
#define max_font_name_length 60
#define MAX_MSGBUTTONS  11

#define __GUI_SEPERATOR__ 	0, 0, 0, 0, 0

#if defined(__APPLE__)
/* Platform FONT type */
typedef NSFont ui_font_t;
/* Platform Window type */
typedef id ui_wnd_t;
/* Platform Color type */
typedef NSColor ui_color_t;
/* Platform Menu type */
typedef NSMenu ui_menu_t;
/* Platform TextField type */
typedef NSTextField ui_form_t;
/* Platform string type */
typedef NSString ui_str_t;
/* Platform bool type */
typedef BOOL ui_bool;
#define ui_field_str(value, field)		ui_str_t value = (ui_str_t)cocoa_send((id)field, "stringValue")
#elif defined(_WIN32)
/* Platform Window type */
typedef HWND ui_wnd_t;
/* Platform FONT type */
typedef HFONT ui_font_t;
/* Platform Color type */
typedef COLORREF ui_color_t;
/* Platform Menu type */
typedef HMENU ui_menu_t;
/* Platform TextField type */
typedef HWND ui_form_t;
/* Platform string type */
typedef char *ui_str_t;
/* Platform bool type */
typedef BOOL ui_bool;
#define ui_field_str(value, field)	\
	char value##[256];				\
	GetWindowTextA(field, value, sizeof(value))
#else
/* Platform Window type */
typedef Widget ui_wnd_t;
/* Platform FONT type */
typedef XFontStruct *ui_font_t;
/* Platform Color type */
typedef Colormap ui_color_t;
/* Platform Menu type */
typedef void *ui_menu_t;
/* Platform TextField type */
typedef Widget ui_form_t;
/* Platform string type */
typedef String ui_str_t;
/* Platform bool type */
typedef bool ui_bool;
typedef struct hist hist_t;
#define ui_field_str(value, field)		ui_str_t value = TextFieldGetString(field)
#endif

typedef union {
	int _int;
	unsigned int _unsigned;
	unsigned long _long_max;
	char _char;
	short _short;
	long _long;
	double _double;
	size_t _size_t;
	char *char_ptr;
	void *ptr;
	intptr_t intptr;
	uintptr_t uintptr;
	ptrdiff_t *ptrdiff;
	const char blob[32];
} ui_values;

typedef struct Forms_s {
	uintptr_t ID;
	ui_field_type kind;
	char *caption;
	char *value;
	int width;
	int max;
	int min;
	ui_form_t index;
} Form;

typedef Button ui_button[MAX_MSGBUTTONS];
typedef Form ui_field;

typedef struct gui_info_s gui_info;
typedef struct {
	/* `Application` main Window handle */
	ui_wnd_t wnd;
	/* App's per `Window` handle */
	void *app_data;
	/* App's per `Window` title */
	const char *name;
#if defined(__APPLE__)
	BOOL running, is_model;
#elif _WIN32
	unsigned long extra;
#endif
	void **app_array;
	gui_info *gui;
	/* App's per `Window` code */
	unsigned long code;
} ui_t;

typedef void (*_platform_cb)(__GUI_FIELD__);
typedef void (*_menu_cb)(__GUI_MENU__);
typedef void (*ui_file_cb)(ui_t *, const char *);
typedef bool (*ui_form_cb)(const ui_t *, uint32_t field_id, void *, char *err);
typedef struct {
	int menu_id;
	char *item_name;
	_menu_cb action;
	char *alphaKey;
	void *data;
#ifdef _WIN32
	HFONT hfont;
	int   cchItemText;
#endif
} menuitem_t;

typedef struct menu_s {
	menuitem_t *items;	// menu items
	int num_items;	// # of menu items
	int active;
	int selected;
	int menu_id;
	char menu_name[32];
	ui_menu_t hMenu;
	ui_font_t hFont;
#if defined(__APPLE__)
#elif defined(_WIN32)
#else
	double x_start;
	double x_end;
	double width;
#endif
} menu_t;

typedef struct {
	int bar_ready;
	int state;
	int num_menus;
	/* main menu */
	menu_t *menus;
#if defined(__APPLE__)
	NSDictionary attr;
#elif defined(_WIN32)
	/* main `window` menu bar */
	long lfHeight;
	int num_fonts;
#else
	XWindowAttributes gwa;
	int num_fonts;
	int *size;
	GLuint font_lists;
#endif
	char *font_names;
	ui_font_t font_info;
	ui_menu_t hMenubar;
} menu_bar_t;

typedef struct webview webview_t;
typedef void (*webview_external_invoke_cb_t)(webview_t *w,
	const char *arg);
struct webview {
	const char *url;
	const char *title;
	int width;
	int height;
	int resizable;
	int showtoolbar;
	int debug;
	webview_external_invoke_cb_t external_invoke_cb;
	struct webview_priv priv;
	ui_wnd_t statusline;
	void *userdata;
};

struct gui_info_s {
	menu_bar_t *bar_info;
	int width;
	int height;
	int x;
	int y;
	int mouse;
	int error;
	int mod;       /* mod is 4 bits mask, ctrl=1, shift=2, alt=4, meta=8 */
	int keys[256]; /* keys are mostly ASCII, but arrows are 17..20 */
	const char *title;
	uint32_t *buf;
	ui_wnd_t wnd;
	ui_font_t font;
	ui_color_t txtCr, bkCr;
	/* For passing data to custom `Window` handler routine */
	void *user_data;
	ui_t app[1];
#if defined(_WIN32) || defined(__APPLE__)
	webview_t web[1];
#endif
#if defined(_WIN32)
	WNDCLASSEX wc;
	MSG msg;
	HINSTANCE hinst;
#elif defined(__APPLE__)
	id pool, appDelObj;
	WKWebView webView[1];
	NSWindow window[1];
	NSTextField statusLine;
	Class delegate;
	id delegate_instance;
#else
	bool icon_set;
	bool skip_resize;
	int screen;
	Colormap cmap;
	Window win, root;
	Atom wmDeleteMessage;
	XEvent xev;
	Widget topLevel, statusLine;
	Display *dpy;
	XVisualInfo *vi;
	XtAppContext app_con;
	GLXContext glc;
	GC gc;
	XImage *img;
	XFontStruct *font_button;
	hist_t *backhist, *forwhist;
#endif
};

C_API size_t str_length(ui_form_t field);
C_API ui_bool str_is_regex(const char *pattern, ui_str_t match);
C_API ui_bool str_field_valid(ui_form_t field, ui_field form);

C_API void gui_open_dialog(__GUI_MENU__);
C_API void gui_save_dialog(__GUI_MENU__);
C_API void gui_file(__GUI_FILE__);
#ifdef __APPLE__
C_API int gui_message_box(id, const char *title, const char *text, const Button *buttons, int numButtons);
#else
C_API int gui_message_box(ui_t *, const char *title, const char *text, const Button *buttons, int numButtons);
#endif
C_API int gui_form(gui_info *, const char *title, Form *fill, int numFields, ui_form_cb verify);
C_API void gui_active(gui_info);
C_API void gui_destroy(gui_info);
C_API int gui_menubar(gui_info *, int numof_menus);
C_API int gui_menufont(gui_info *, const char *font);
C_API int gui_menu(gui_info *, int num_menu, menuitem_t *items, int number_items, int menu_id, char *name);
C_API int gui_handler(gui_info *);
C_API void gui_cancel(ui_wnd_t);
C_API ui_wnd_t gui_statusline(ui_wnd_t, ui_wnd_t alignto, const char *message, int y, int width);

C_API int gui_window(gui_info *, const char *title, int width, int height, int buffered);
C_API int gui_loop(gui_info *);
C_API void gui_close(gui_info *);
C_API void gui_sleep(int64_t ms);
C_API int64_t gui_time(void);

#define gui_pixel(w, x, y) ((w)->buf[((y) * (w)->width) + (x)])
#ifndef trace
#	define Statement(s) do {	\
			s	\
		}	while (0)
#	define trace 		Statement(fprintf(stderr, "%s:%d Trace \n", __FILE__, __LINE__);)
#endif

#ifndef casting
	/* Cast ~val~, a `non-pointer` to `pointer` like value,
	makes reference if variable. */
#	define casting(val) (void *)((ptrdiff_t)(val))
#endif

C_API int gui_webview(gui_info *ui, const char *title, const char *url, int width, int height, bool showtoolbar);
C_API void gui_webactive(gui_info ui);
C_API void gui_webdestroy(gui_info ui);

C_API ui_bool str_is_regex(const char *pattern, ui_str_t match);
C_API ui_bool is_ValidUrl(ui_str_t text);
C_API ui_bool is_ValidEmail(ui_str_t text);
C_API ui_bool is_ValidPassword(ui_form_t field);
C_API size_t str_length(ui_form_t field);

#define DEFAULT_URL                                                            \
  "data:text/"                                                                 \
  "html,%3C%21DOCTYPE%20html%3E%0A%3Chtml%20lang=%22en%22%3E%0A%3Chead%3E%"    \
  "3Cmeta%20charset=%22utf-8%22%3E%3Cmeta%20http-equiv=%22X-UA-Compatible%22%" \
  "20content=%22IE=edge%22%3E%3C%2Fhead%3E%0A%3Cbody%3E%3Cdiv%20id=%22app%22%" \
  "3E%3C%2Fdiv%3E%3Cscript%20type=%22text%2Fjavascript%22%3E%3C%2Fscript%3E%"  \
  "3C%2Fbody%3E%0A%3C%2Fhtml%3E"

#define CSS_INJECT_FUNCTION                                                    \
  "(function(e){var "                                                          \
  "t=document.createElement('style'),d=document.head||document."               \
  "getElementsByTagName('head')[0];t.setAttribute('type','text/"               \
  "css'),t.styleSheet?t.styleSheet.cssText=e:t.appendChild(document."          \
  "createTextNode(e)),d.appendChild(t)})"
#define WEBVIEW_API C_API

/*
 * MIT License
 *
 * For https://github.com/zserge/webview
 *
 * Copyright (c) 2017 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
enum webview_dialog_type {
	WEBVIEW_DIALOG_TYPE_OPEN = 0,
	WEBVIEW_DIALOG_TYPE_SAVE = 1,
	WEBVIEW_DIALOG_TYPE_ALERT = 2
};

enum webview_dialog_flag {
	WEBVIEW_DIALOG_FLAG_FILE = (0 << 0),
	WEBVIEW_DIALOG_FLAG_DIRECTORY = (1 << 0),

	WEBVIEW_DIALOG_FLAG_INFO = (1 << 1),
	WEBVIEW_DIALOG_FLAG_WARNING = (2 << 1),
	WEBVIEW_DIALOG_FLAG_ERROR = (3 << 1),
	WEBVIEW_DIALOG_FLAG_ALERT_MASK = (3 << 1),
};

typedef void (*webview_dispatch_fn)(webview_t *w, void *arg);
typedef struct webview_dispatch_arg webview_dispatch_arg;

struct webview_dispatch_arg {
	webview_dispatch_fn fn;
	webview_t *w;
	void *arg;
};

WEBVIEW_API int webview_run(const char *title, const char *url, int width, int height);

WEBVIEW_API int webview_loop(webview_t *w, int blocking);
WEBVIEW_API int webview_eval(webview_t *w, const char *js);
WEBVIEW_API int webview_inject_css(webview_t *w, const char *css);
WEBVIEW_API void webview_set_fullscreen(webview_t *w, int fullscreen);
WEBVIEW_API void webview_set_color(webview_t *w, uint8_t r, uint8_t g,
	uint8_t b, uint8_t a);
WEBVIEW_API void webview_dialog(webview_t *w,
	enum webview_dialog_type dlgtype, int flags,
	const char *title, const char *arg,
	char *result, size_t resultsz);
WEBVIEW_API void webview_dispatch(webview_t *w, webview_dispatch_fn fn,
	void *arg);
WEBVIEW_API void webview_exit(webview_t *w);
WEBVIEW_API void webview_debug(const char *format, ...);
WEBVIEW_API void webview_print_log(const char *s);

WEBVIEW_API int webview_create(gui_info *ui, webview_t *w);
WEBVIEW_API void webview_loadfile(webview_t *w, const char *resourcefile, const char *type);

/* Returns a native window handle pointer. */
WEBVIEW_API ui_wnd_t webview_get_window(webview_t *w);

/* Updates the title of the native window. Must be called from the UI thread. */
WEBVIEW_API void webview_set_title(webview_t *w, const char *title);

// Window size hints
#define WEBVIEW_HINT_NONE 0  // Width and height are default size
#define WEBVIEW_HINT_MIN 1   // Width and height are minimum bounds
#define WEBVIEW_HINT_MAX 2   // Width and height are maximum bounds
#define WEBVIEW_HINT_FIXED 3 // Window size can not be changed by a user

/* Updates native window size. See WEBVIEW_HINT constants. */
WEBVIEW_API void webview_set_size(webview_t *w, int width, int height, int hints);

/* Navigates webview to the given URL. URL may be a properly encoded data URI.
 *
 * Examples:
 * `webview_navigate(w, "https://github.com/webview/webview");`
 * `webview_navigate(w, "data:text/html,%3Ch1%3EHello%3C%2Fh1%3E");`
 * `webview_navigate(w, "data:text/html;base64,PGgxPkhlbGxvPC9oMT4=");` */
WEBVIEW_API void webview_navigate(webview_t *w, const char *url);

/* Set webview HTML directly.
 *
 * Example: `webview_set_html(w, "<h1>Hello</h1>");` */
WEBVIEW_API void webview_set_html(webview_t *w, const char *url);

WEBVIEW_API void webview_go_to(__GUI_WEBVIEW__);

// Go home
WEBVIEW_API void webview_home(__GUI_WEBVIEW__);

// Go back
WEBVIEW_API void webview_go_back(__GUI_WEBVIEW__);

// Go forward
WEBVIEW_API void webview_go_forward(__GUI_WEBVIEW__);

// Reload page
WEBVIEW_API void webview_reload(__GUI_WEBVIEW__);

// Stop loading page
WEBVIEW_API void webview_stop(__GUI_WEBVIEW__);

// Get current page title
WEBVIEW_API char *webview_get_title(webview_t *w);

// Get current page URL
WEBVIEW_API char *webview_get_url(webview_t *w);
#endif /* _GUI_H */
