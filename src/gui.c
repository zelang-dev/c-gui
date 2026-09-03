#include <gui.h>
#include "libsmallregex.h"
static volatile gui_info *main_gui_info = NULL;
static volatile bool main_gui_shutdown = false;

#if defined(__linux__) && !defined(X11_WINDOW_ICON)
#define X11_WINDOW_ICON "../resources/icon_32x32.xpm"
#endif

FORCEINLINE size_t str_length(ui_form_t field) {
#if __APPLE__
	return cocoa_strlen((NSString)cocoa_send((id)field, "stringValue"));
#elif defined(__linux__)
	size_t len = 0;
	String value = TextFieldGetString(field);
	if (value) {
		len = strlen(value);
		free(value);
	}
	return len;
#else
	return GetWindowTextLength(field);
#endif
}

FORCEINLINE ui_bool str_is_regex(const char *pattern, ui_str_t match) {
#if __APPLE__
	return regex_match(pattern, (const char *)cocoa_tochar(match)) == 0;
#else
	return regex_match(pattern, (const char *)match) == 0;
#endif
}

FORCEINLINE ui_bool is_ValidEmail(ui_str_t text) {
	const char *emailRegEx = "A[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9]"
		"(?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?";
	return str_is_regex(emailRegEx, text);
}

FORCEINLINE ui_bool is_ValidUrl(ui_str_t text) {
	return str_is_regex("(http|https)://((\\w)*|([0-9]*)|([-|_])*)+([\\.|/]((\\w)*|([0-9]*)|([-|_])*))+", text);
}

ui_bool is_ValidPassword(ui_form_t field) {
	ui_field_str(passwordValue, field);
	ui_bool capitalResult = str_is_regex(".*[A-Z]+.*", passwordValue);
	ui_bool smallResult = str_is_regex(".*[a-z]+.*", passwordValue);
	ui_bool numberResult = str_is_regex(".*[0-9]+.*", passwordValue);

	return capitalResult && smallResult && numberResult;
}

static FORCEINLINE ui_bool is_MinLength(ui_form_t text, ui_field form) {
	return (0 == form.min) ? (ui_bool)true : (int)str_length(text) >= form.min;
}

static FORCEINLINE ui_bool is_MaxLength(ui_form_t text, ui_field form) {
	return (0 == form.max) ? (ui_bool)true : (int)str_length(text) <= form.max;
}

static FORCEINLINE ui_bool is_EmailValid(ui_form_t field, ui_field form) {
	ui_field_str(value, field);
	return form.kind == field_email ? is_ValidEmail(value) : (ui_bool)true;
}

static FORCEINLINE ui_bool is_UrlValid(ui_form_t field, ui_field form) {
	ui_field_str(value, field);
	return form.kind == field_url ? is_ValidUrl(value) : (ui_bool)true;
}

static FORCEINLINE ui_bool is_PasswordValid(ui_form_t text, ui_field form) {
	return (form.kind == field_secret) ? is_ValidPassword(text) : (ui_bool)true;
}

FORCEINLINE ui_bool str_field_valid(ui_form_t field, ui_field form) {
	ui_bool minimunLengthValidly = is_MinLength(field, form);
	ui_bool maximumLengthValidity = is_MaxLength(field, form);
	ui_bool emailValidity = is_EmailValid(field, form);
	ui_bool passwordValidity = is_PasswordValid(field, form);
	ui_bool urlValidity = is_UrlValid(field, form);
	return (minimunLengthValidly && maximumLengthValidity && emailValidity && passwordValidity && urlValidity);
}
#if defined(__APPLE__)
static volatile bool main_apple_menu_ready = false;
// This is equivalent to creating a @class with one public variable named 'window'.
// This is a strong reference to the class of the AppDelegate
// (same as [AppDelegate class])
Class AppDelClass = NULL;

cocoa_window_cb cocoa_window_func = (cocoa_window_cb)objc_msgSend;
cocoa_rect_cb cocoa_rect_func = (cocoa_rect_cb)objc_msgSend;
cocoa_menu_cb cocoa_menu_func = (cocoa_menu_cb)objc_msgSend;
cocoa_event_cb cocoa_event_func = (cocoa_event_cb)objc_msgSend;
cocoa_send_cb cocoa_send_func = (cocoa_send_cb)objc_msgSend;
cocoa_sendclass_cb cocoa_sendclass_func = (cocoa_sendclass_cb)objc_msgSend;
cocoa_sendrect_cb cocoa_sendrect_func = (cocoa_sendrect_cb)objc_msgSend;
cocoa_sendview_cb cocoa_sendview_func = (cocoa_sendview_cb)objc_msgSend;
cocoa_sendany_cb cocoa_sendany_func = (cocoa_sendany_cb)objc_msgSend;
cocoa_sendwith_cb cocoa_sendwith_func = (cocoa_sendwith_cb)objc_msgSend;
cocoa_sendfloat_cb cocoa_sendfloat_func = (cocoa_sendfloat_cb)objc_msgSend;
cocoa_sendvariadic_cb cocoa_sendvariadic_func = (cocoa_sendvariadic_cb)objc_msgSend;
cocoa_sendpair_cb cocoa_sendpair_func = (cocoa_sendpair_cb)objc_msgSend;
cocoa_sendwithint_cb cocoa_sendwithint_func = (cocoa_sendwithint_cb)objc_msgSend;
cocoa_intwith_cb cocoa_intwith_func = (cocoa_intwith_cb)objc_msgSend;
cocoa_range_cb cocoa_range_func = (cocoa_range_cb)objc_msgSend;
cocoa_sendwithpair_cb cocoa_sendwithpair_func = (cocoa_sendwithpair_cb)objc_msgSend;
cocoa_sendint_cb cocoa_sendint_func = (cocoa_sendint_cb)objc_msgSend;
cocoa_int_cb cocoa_int_func = (cocoa_int_cb)objc_msgSend;
cocoa_intpair_cb cocoa_intpair_func = (cocoa_intpair_cb)objc_msgSend;
cocoa_size_cb cocoa_size_func = (cocoa_size_cb)objc_msgSend;

cocoa_postpoint_cb cocoa_postpoint_func = (cocoa_postpoint_cb)objc_msgSend;
cocoa_postsize_cb cocoa_postsize_func = (cocoa_postsize_cb)objc_msgSend;
cocoa_postrect_cb cocoa_postrect_func = (cocoa_postrect_cb)objc_msgSend;
cocoa_postrectint_cb cocoa_postrectint_func = (cocoa_postrectint_cb)objc_msgSend;
cocoa_postint_cb cocoa_postint_func = (cocoa_postint_cb)objc_msgSend;
cocoa_postany_cb cocoa_postany_func = (cocoa_postany_cb)objc_msgSend;
cocoa_postimp_cb cocoa_postimp_func = (cocoa_postimp_cb)objc_msgSend;
cocoa_postsel_cb cocoa_postsel_func = (cocoa_postsel_cb)objc_msgSend;
cocoa_postpair_cb cocoa_postpair_func = (cocoa_postpair_cb)objc_msgSend;
cocoa_postrectwith_cb cocoa_postrectwith_func = (cocoa_postrectwith_cb)objc_msgSend;
cocoa_postpairwith_cb cocoa_postpairwith_func = (cocoa_postpairwith_cb)objc_msgSend;
cocoa_postwithint_cb cocoa_postwithint_func = (cocoa_postwithint_cb)objc_msgSend;;
cocoa_postwithtwo_cb cocoa_postwithtwo_func = (cocoa_postwithtwo_cb)objc_msgSend;;
cocoa_postid_cb cocoa_postid_func = (cocoa_postid_cb)objc_msgSend;
cocoa_post_cb cocoa_post_func = (cocoa_post_cb)objc_msgSend;
cocoa_model_cb cocoa_model_func = (cocoa_model_cb)objc_msgSend;
cocoa_modelint_cb cocoa_modelint_func = (cocoa_modelint_cb)objc_msgSend;
cocoa_postnotification_cb cocoa_postnotification_func = (cocoa_postnotification_cb)objc_msgSend;

static const NSModalResponse NSModalResponseOK = 1;
static const NSModalResponse NSModalResponseCancel = 0;

FORCEINLINE id cocoa_get(const char *id_class, const char *selector) {
	return cocoa_send_func((id)objc_getClass(id_class), sel_getUid(selector));
}

FORCEINLINE id cocoa_get_with(const char *id_class, const char *selector, id with) {
	return cocoa_sendclass_func((id)objc_getClass(id_class), sel_getUid(selector), with);
}

FORCEINLINE id cocoa_get_status(const char *id_class, const char *selector, NSInteger code) {
	return cocoa_sendint_func((id)objc_getClass(id_class), sel_getUid(selector), code);
}

FORCEINLINE void cocoa_post(const char *id_class, const char *selector) {
	cocoa_post_func((id)objc_getClass(id_class), sel_getUid(selector));
}

FORCEINLINE id cocoa_send(id instance, const char *selector) {
	return cocoa_send_func(instance, sel_getUid(selector));
}

FORCEINLINE NSInteger cocoa_status(id instance, const char *selector) {
	return cocoa_int_func(instance, sel_getUid(selector));
}

FORCEINLINE NSInteger cocoa_status_with(id instance, const char *selector, id with, id self) {
	return cocoa_intpair_func(instance, sel_getUid(selector), with, self);
}

FORCEINLINE id cocoa_alloc(const char *id_class) {
	return cocoa_get(id_class, "alloc");
}

FORCEINLINE id cocoa_new(const char *id_class) {
	return cocoa_get(id_class, "new");
}

FORCEINLINE id cocoa_autorelease(const char *id_class) {
	return cocoa_send(cocoa_alloc(id_class), "autorelease");
}

FORCEINLINE void cocoa_select(id instance, const char *selector) {
	cocoa_post_func(instance, sel_getUid(selector));
}

FORCEINLINE void cocoa_select_handler(id instance, const char *selector, id self, IMP_INT func) {
	cocoa_modelint_func(instance, sel_getUid(selector), self, func);
}

FORCEINLINE void cocoa_set_rect(id instance, const char *selector, float x, float y, float width, float height) {
	cocoa_postrect_func(instance, sel_getUid(selector), CGRectMake(x, y, width, height));
}

FORCEINLINE void cocoa_set_point(id instance, const char *selector, float x, float y) {
	cocoa_postpoint_func(instance, sel_getUid(selector), CGPointMake(x, y));
}

FORCEINLINE void cocoa_set_size(id instance, const char *selector, float x, float y) {
	cocoa_postsize_func(instance, sel_getUid(selector), CGSizeMake(x, y));
}

FORCEINLINE void cocoa_set_with(id instance, const char *selector, id with) {
	cocoa_postid_func(instance, sel_getUid(selector), with);
}

FORCEINLINE void cocoa_set_pair(id instance, const char *selector, id with, id pair) {
	cocoa_postpair_func(instance, sel_getUid(selector), with, pair);
}

FORCEINLINE void cocoa_set(id instance, const char *selector, int value) {
	cocoa_postint_func(instance, sel_getUid(selector), value);
}

FORCEINLINE id cocoa_send_data(id instance, const char *selector, void *data) {
	return cocoa_sendany_func(instance, sel_getUid(selector), data);
}

FORCEINLINE id cocoa_send_with(id instance, const char *selector, id data) {
	return cocoa_send_data(instance, selector, (void *)data);
}

FORCEINLINE id cocoa_send_status(id instance, const char *selector, NSInteger code) {
	return cocoa_sendint_func((id)instance, sel_getUid(selector), code);
}

FORCEINLINE id cocoa_send_rect(id instance, const char *selector, float x, float y, float width, float height) {
	return cocoa_sendrect_func(instance, sel_getUid(selector), CGRectMake(x, y, width, height));
}

FORCEINLINE id cocoa_init(id instance) {
	return cocoa_send(instance, "init");
}

FORCEINLINE id cocoa_alloc_class(Class object) {
	return cocoa_send((id)object, "alloc");
}

FORCEINLINE Class cocoa_constructor(const char *superclass, const char *alloc_class,
	const char *sel_name, IMP imp, const char *types) {
	Class new_class = objc_allocateClassPair(objc_getClass(superclass), alloc_class, 0);
	class_addMethod(new_class, sel_getUid(sel_name), imp, types);
	objc_registerClassPair(new_class);
	return new_class;
}

NSButton cocoa_button(id window, char *label, char *action, float x, float y, float width,
	NSBezelStyle style, int location) {
	NSAutoresizingMaskOptions resize_mask = location != -1
		? NSViewMaxXMargin | NSViewMaxYMargin
		: NSViewMinXMargin;
	NSButton button = cocoa_send(cocoa_send_rect(cocoa_alloc("NSButton"), "initWithFrame:", x, y, width, 19), "autorelease");
	cocoa_set_with(button, "setTitle:", (id)cocoa_str(label));
	cocoa_set(button, "setBezelStyle:", style);
	cocoa_postsel_func(button, sel_registerName("setAction:"), sel_getUid(action));
	cocoa_set_with(button, "setTarget:", nil);
	cocoa_set(button, "setAutoresizingMask:", (location == YES
		? resize_mask | NSViewMinYMargin
		: (location == -1
			? resize_mask | NSViewMaxYMargin | NSViewMinYMargin
			: resize_mask)));
	cocoa_set_with(cocoa_send(window, "contentView"), "addSubview:", button);
	return button;
}

NSTextField cocoa_field(id window, id alignwith, char *inital, float x, float y, float width,
	ui_field_type kind) {
	NSTextField field = cocoa_send(cocoa_send_rect((kind == field_secret
		? cocoa_alloc("NSSecureTextField") : cocoa_alloc("NSTextField")),
		"initWithFrame:", x, y, width, 20), "autorelease");
	cocoa_select(field, "becomeFirstResponder");
	cocoa_set(field, "setAutoresizingMask:", (NSViewWidthSizable | NSViewMaxYMargin | NSViewMinYMargin));
	cocoa_set_with(field, "setStringValue:", (id)cocoa_str(inital));
	cocoa_postpairwith_func(cocoa_send(window, "contentView"),
		sel_getUid("addSubview:positioned:relativeTo:"), field, NSWindowAbove, alignwith);
	return field;
}

FORCEINLINE void cocoa_textcolor(id view, CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha) {
	cocoa_set_with(cocoa_send(view, "cell"), "setTextColor:",
		(id)CGColorCreateGenericRGB(red, green, blue, alpha));
}

FORCEINLINE void cocoa_backgroundcolor(id view, CGFloat red, CGFloat green, CGFloat blue, CGFloat alpha) {
	cocoa_set_with(cocoa_send(view, "cell"), "setBackgroundColor:",
		(id)CGColorCreateGenericRGB(red, green, blue, alpha));
}

FORCEINLINE void cocoa_cell_colors(id view, const char *text, const char *background) {
	if (view) {
		if (text)
			cocoa_set_with(cocoa_send(view, "cell"), "setTextColor:", cocoa_get("NSColor", text));
		if (background)
			cocoa_set_with(cocoa_send(view, "cell"), "setBackgroundColor:", cocoa_get("NSColor", background));
	}
}

FORCEINLINE id cocoa_window(int x, int y, int width, int height, int style, int backing, bool defer) {
	return cocoa_window_func(cocoa_alloc("NSWindow"),
		sel_getUid("initWithContentRect:styleMask:backing:defer:"),
		CGRectMake(x, y, width, height), style, backing, (BOOL)defer);
}

FORCEINLINE NSEvent cocoa_next_event(id instance, unsigned long mask, id expiration, id mode, BOOL deqFlag) {
	return cocoa_event_func(instance, sel_getUid("nextEventMatchingMask:untilDate:inMode:dequeue:"), mask, expiration, mode, deqFlag);
}

FORCEINLINE NSString cocoa_str(const char *text) {
	return (NSString)cocoa_send_data((id)objc_getClass("NSString"), "stringWithUTF8String:", (void *)text);
}

FORCEINLINE NSInteger cocoa_strlen(NSString str) {
	return cocoa_status((id)str, "length");
}

FORCEINLINE NSString cocoa_sprintf(const char *fmt, ...) {
	va_list arguments;

	va_start(arguments, fmt);
	id result = cocoa_send(cocoa_sendvariadic_func(cocoa_get_with("NSString", "allocWithZone:", nil),
		sel_getUid("initWithFormat:arguments:"), (id)cocoa_str(fmt), arguments), "autorelease");
	va_end(arguments);
	return (NSString)result;
}

FORCEINLINE char *cocoa_tochar(NSString str) {
	return (char *)cocoa_send((id)str, "UTF8String");
}

FORCEINLINE NSMutableArray cocoa_array_mutable(void) {
	return (NSMutableArray)cocoa_autorelease("NSMutableArray");
}

FORCEINLINE void cocoa_append(NSMutableArray arr, id value) {
	cocoa_set_with((id)arr, "addObject:", value);
}

NSArray cocoa_array(id instance, ...) {
	NSUInteger i, count = 0;
	id *objects = NULL;
	va_list  arguments;

	if (instance != nil) {
		va_start(arguments, instance);
		count = 1; // include object
		while (va_arg(arguments, id) != nil)
			count++;
		va_end(arguments);

		objects = __builtin_alloca(sizeof(id) * count);
		va_start(arguments, instance);
		objects[0] = instance;
		for (i = 1;i < count;i++)
			objects[i] = va_arg(arguments, id);
		va_end(arguments);
	}

	return (NSArray)cocoa_send(cocoa_sendwith_func((id)objc_getClass("object"),
		sel_getUid("arrayWithObjects:count"), objects, count), "autorelease");
}

FORCEINLINE id cocoa_array_index(NSArray arr, NSInteger at) {
	return cocoa_sendint_func((id)arr, sel_getUid("objectAtIndex:"), at);
}

FORCEINLINE NSInteger cocoa_array_count(NSArray arr) {
	return (NSInteger)cocoa_int_func((id)arr, sel_getUid("count"));
}

FORCEINLINE NSDictionary cocoa_dict(NSString key, id value) {
	return (NSDictionary)cocoa_sendpair_func((id)objc_getClass("NSDictionary"),
		sel_getUid("dictionaryWithObject:forKey:"), value, (id)key);
}

NSDictionary cocoa_dictionary(id value, char *key, ...) {
	va_list  arguments;
	NSUInteger i, count;
	id *objects, *keys;

	va_start(arguments, key);
	count = 1;
	while (va_arg(arguments, id) != nil)
		count++;
	va_end(arguments);

	objects = __builtin_alloca(sizeof(id) * count / 2);
	keys = __builtin_alloca(sizeof(id) * count / 2);

	objects[0] = value;
	keys[0] = (id)cocoa_str(key);

	va_start(arguments, key);
	for (i = 1;i < count / 2;i++) {
		objects[i] = va_arg(arguments, id);
		keys[i] = (id)cocoa_str((const char *)va_arg(arguments, char *));
	}
	va_end(arguments);

	return (NSDictionary)cocoa_send(cocoa_sendwithpair_func((id)objc_getClass("NSDictionary"),
		sel_getUid("initWithObjects:forKeys:count:"), (id)objects, (id)keys, (count / 2)), "autorelease");
}

void cocoa_impl_func(const char *class_name, const char *register_name, void *function) {
	Class selected_class;

	if (strcmp(class_name, "NSView") == 0) {
		selected_class = objc_getClass("ViewClass");
	} else if (strcmp(class_name, "NSWindow") == 0) {
		selected_class = objc_getClass("WindowClass");
	} else {
		selected_class = objc_getClass(class_name);
	}

	class_addMethod(selected_class, sel_registerName(register_name), (IMP)function, "i@:@");
}

FORCEINLINE NSFont cocoa_font(const char *fontName, float size) {
	return (NSFont)cocoa_sendfloat_func((id)objc_getClass("NSFont"),
		sel_getUid("fontWithName:size:"), (id)cocoa_str(fontName), size);
}

FORCEINLINE void cocoa_menuitem_font(NSMenuItem menuItem, NSDictionary attributes) {
	NSAttributedString attributedTitle = (NSAttributedString)cocoa_sendpair_func(cocoa_alloc("NSAttributedString"),
		sel_getUid("initWithString:attributes:"), cocoa_send(menuItem, "title"), (id)attributes);
	cocoa_set_with(menuItem, "setAttributedTitle:", (id)attributedTitle);
}

NSMenuItem cocoa_menuitem_action(id menu, id append, char *name, void *action, char *key, void *object) {
	id app_name, title = (id)cocoa_str(name);
	app_name = title;;
	if (append)
		app_name = cocoa_send_with(title, "stringByAppendingString:", append);

	char selector[64] = {0};
	snprintf(selector, 63, "%p", (IMP)action);
	cocoa_impl_func("NSObject", selector, action);
	NSMenuItem item = cocoa_menu_func(cocoa_autorelease("NSMenuItem"), sel_getUid("initWithTitle:action:keyEquivalent:"),
		(NSString *)app_name, sel_registerName(selector), (NSString *)cocoa_str(key));

	cocoa_set_with(item, "setRepresentedObject:", (id)object);
	cocoa_set_with(menu, "addItem:", item);
	return item;
}

FORCEINLINE void cocoa_menu_separator(id menu) {
	cocoa_set_with(menu, "addItem:", cocoa_get("NSMenuItem", "separatorItem"));
}

FORCEINLINE void cocoa_menuitem_sub(id menu, const char *name, id sub) {
	id item = cocoa_menu_func(cocoa_autorelease("NSMenuItem"), sel_getUid("initWithTitle:action:keyEquivalent:"),
		(NSString *)cocoa_str(name), sel_registerName(""), (NSString *)cocoa_str(""));
	cocoa_set_with(menu, "addItem:", item);
	cocoa_set_with(item, "setSubmenu:", sub);
}

FORCEINLINE void cocoa_menuitem(id menu, id append, const char *name, const char *selector, const char *key) {
	id item, app_name, title = (id)cocoa_str(name);
	app_name = title;
	if (append != NULL)
		app_name = cocoa_send_with(title, "stringByAppendingString:", append);

	item = cocoa_menu_func(cocoa_autorelease("NSMenuItem"), sel_getUid("initWithTitle:action:keyEquivalent:"),
		(NSString *)app_name, sel_registerName(selector), (NSString *)cocoa_str(key));
	cocoa_set_with(menu, "addItem:", item);
}

FORCEINLINE NSRange cocoa_str_pos(NSString str, char *match, NSInteger options) {
	return cocoa_range_func((id)str, sel_getUid("rangeOfString:options:"),
		(id)cocoa_str(match), options);
}

FORCEINLINE BOOL cocoa_str_has(NSString str, char *match) {
	return cocoa_str_pos(str, match, NSCaseInsensitiveSearch).location != NSNotFound;
}

FORCEINLINE BOOL cocoa_str_is(NSString str, char *match) {
	return (BOOL)cocoa_intwith_func((id)str, sel_getUid("isEqualToString:"), (id)cocoa_str(match));
}

static FORCEINLINE void terminate_handler(__GUI_MENU__) {
	if (!main_gui_shutdown && !main_gui_info->bar_info && main_gui_info->web->priv.webview){
		main_gui_shutdown = true;
		cocoa_select(main_gui_info->window[0], "close");
		main_gui_info->web->priv.should_exit = 1;
	} else if (!main_gui_shutdown) {
		main_gui_shutdown = true;
		if (main_gui_info->window[0])
			cocoa_select(main_gui_info->window[0], "close");

		cocoa_select(main_gui_info->wnd, "close");
		cocoa_set_with(NSApp, "stop:", nil);
	} else {
		cocoa_set_with(NSApp, "terminate:", self);
	}
}

static FORCEINLINE BOOL should_close(__GUI_MENU__) {
	(void)selector;
	(void)data;
	BOOL is_closing = NO;
	gui_info *ui = nil;
	object_getInstanceVariable(self, "gui_info", (void *)&ui);
	if (!ui) {
		main_gui_shutdown = true;
		return YES;
	}

	if (ui == main_gui_info) {
		is_closing = YES;
		ui->app->running = NO;
		main_gui_shutdown = true;
	} else if (ui->app->running) {
		ui->app->running = NO;
		is_closing = main_gui_shutdown;
	}

	return is_closing;
}

static FORCEINLINE void reset_form(__GUI_MENU__) {
	(void)data;
	(void)selector;
	(void)self;
}

static FORCEINLINE void cancel_form(__GUI_MENU__) {
	(void)data;
	(void)selector;
	gui_info *ui = nil;
	object_getInstanceVariable(self, "gui_info", (void *)&ui);
	ui->app->running = NO;
	cocoa_select(ui->app->wnd, "close");
}

static void verify_form(__GUI_FIELD__) {
	(void)data;
	(void)selector;
	gui_info *ui = nil;
	object_getInstanceVariable(self, "gui_info", (void *)&ui);
	ui_field which, *form = (ui_field *)ui->app->app_array;
	NSColor clr_red = CGColorCreateGenericRGB(RGB_RED, 0);
	int numFields = ui->app->code, i, len;
	NSTextField field = nil;

	for (i = 0; i < numFields; i++) {
		which = (ui_field)form[i];
		field = (NSTextField)which.index;
		char error[100] = {0};
		if (str_field_valid(field, which)) {
			cocoa_set_with(ui->statusLine, "setStringValue:", (id)cocoa_str(""));
			cocoa_set_with(cocoa_send((id)which.index, "cell"), "setBackgroundColor:", cocoa_get("NSColor", "greenColor"));
		} else {
			switch (which.kind) {
				case field_number:
					snprintf(error, sizeof(error), "Error: number must be between %d or %d digits", which.min, which.max);
					break;
				case field_text:
					snprintf(error, sizeof(error), "Error: length overflow %d or underflow %d", which.max, which.min);
					break;
				case field_secret:
					snprintf(error, sizeof(error), "Error: secret aleast 1 cap, 1 number and minimum %d characters", which.min);
					break;
				case field_email:
					snprintf(error, sizeof(error), "Error: invalid Email");
					break;
				case field_url:
					snprintf(error, sizeof(error), "Error: invalid Url");
					break;
				case field_date:
					snprintf(error, sizeof(error), "Error: invalid Date");
					break;
				case field_regex:
					snprintf(error, sizeof(error), "Error: invalid regex");
					break;
				default:
					snprintf(error, sizeof(error), "Error: unknown data");
					break;
			}

			cocoa_set_with(ui->statusLine, "setStringValue:", (id)cocoa_str(error));
			cocoa_set_with(cocoa_send(ui->statusLine, "cell"), "setTextColor:", cocoa_get("NSColor", "redColor"));
			cocoa_set_with(cocoa_send((id)which.index, "cell"), "setTextColor:", cocoa_get("NSColor", "whiteColor"));
			cocoa_set_with(cocoa_send((id)which.index, "cell"), "setBackgroundColor:", cocoa_get("NSColor", "redColor"));
			return;
		}
	}
	ui->app->running = NO;
	cocoa_select(ui->app->wnd, "close");
}

static BOOL AppDel_shouldTerminateAfterLastWindowClosed(__GUI_FIELD__) {
	return YES;
}

static BOOL AppDel_didFinishLaunching(AppDelegate *self, SEL selector, id data) {
	(void)selector;
	(void)data;

	self->window = main_gui_info->wnd;
	// Application menu
	id menubar = cocoa_send(cocoa_new("NSMenu"), "autorelease");
	id appMenuItem = cocoa_send(cocoa_new("NSMenuItem"), "autorelease");
	id appMenu = cocoa_send(cocoa_new("NSMenu"), "autorelease");

	cocoa_set_with(menubar, "addItem:", appMenuItem);
	cocoa_set_with(NSApp, "setMainMenu:", menubar);
	cocoa_set_with(NSApp, "setServicesMenu:",
		cocoa_send_with(cocoa_alloc("NSMenu"), "initWithTitle:", (id)cocoa_str("Services")));

	id appName = cocoa_send(cocoa_get("NSProcessInfo", "processInfo"), "processName");
	cocoa_menuitem(appMenu, appName, "About ", "orderFrontStandardAboutPanel:", "");
	cocoa_menu_separator(appMenu);

	/* Services: todo */
	cocoa_menuitem_sub(appMenu, "Services", cocoa_send(NSApp, "servicesMenu"));
	cocoa_menu_separator(appMenu);

	cocoa_menuitem(appMenu, appName, "Hide ", "hide:", "h");
	cocoa_menuitem(appMenu, nil, "Hide Other", "hideOtherApplications:", "h");
	cocoa_menuitem(appMenu, nil, "Show All", "unhideAllApplications:", "");
	cocoa_menu_separator(appMenu);

	cocoa_menuitem_action(appMenu, appName, "Quit ", terminate_handler, "q", nil);
	cocoa_set_with(appMenuItem, "setSubmenu:", appMenu);

	cocoa_set_with(NSApp, "stop:", nil);
	return YES;
}

static void webView_didFinishNavigation(__GUI_FIELD__) {
#ifdef USE_DEBUG
	fprintf(stderr, "[ObjC]\t\t\tdidFinishNavigation()\n");
#endif
	NSString theTitle = (NSString)cocoa_send(data, "title");
	cocoa_set_with(main_gui_info->window[0], "setTitle:", (id)theTitle);
#ifdef USE_DEBUG
	fprintf(stderr, "[ObjC]\t\t\tdidFinishNavigation() -> Updated title [%s]\n", cocoa_tochar(theTitle));
#endif
}

static void webView_windowWillClose(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w) {
		w->priv.should_exit = 1;
		main_gui_info->web->priv.should_exit = 1;
	}
}

static void webView_didReceiveScriptMessage(__GUI_FIELD__, id message) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(data, "webview");
	if (w) {
		NSString handler = (NSString)cocoa_send(message, "name");
		if (cocoa_str_is(handler, "invoke")) {
			if (w->external_invoke_cb)
				w->external_invoke_cb(w, cocoa_tochar((NSString)cocoa_send(message, "body")));
		} else if (cocoa_str_is(handler, "newUrlDetected")) {
			cocoa_set_with(w->statusline, "setStringValue:", (id)cocoa_send(message, "body"));
		}
	}
}

#include "webview-cocoa.c"

static void cocoa_application(gui_info *ui) {
	if (AppDelClass == NULL) {
		AppDelClass = cocoa_constructor("NSObject", "AppDelegate", "applicationDidFinishLaunching:",
			(IMP)AppDel_didFinishLaunching, "i@:@");
		class_addMethod(AppDelClass, sel_getUid("applicationShouldTerminateAfterLastWindowClosed:"),
			(IMP)AppDel_shouldTerminateAfterLastWindowClosed, "i@:@");

		cocoa_post("NSApplication", "sharedApplication");
		if (NSApp == NULL) {
			fprintf(stderr, "Failed to initialized NSApplication...  terminating...\n");
			return;
		}

		/// Create an instance of the window.
		ui->wnd = cocoa_window(0, ui->height, ui->width, ui->height,
			(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
				| NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable),
			NSBackingStoreBuffered, YES);

		main_gui_info->wnd = ui->wnd;
		cocoa_select(ui->wnd, "becomeFirstResponder");
		cocoa_set_with(ui->wnd, "makeKeyAndOrderFront:", nil);
		cocoa_set(ui->wnd, "setAutorecalculatesKeyViewLoop:", YES);
		cocoa_set(NSApp, "setActivationPolicy:", NSApplicationActivationPolicyRegular);
		ui->appDelObj = cocoa_init(cocoa_alloc("AppDelegate"));
		cocoa_set_with(NSApp, "setDelegate:", ui->appDelObj);
		cocoa_select(NSApp, "run");
		ui->webView[0] = nil;
		ui->window[0] = nil;
	}
}

static void gui_window_resize(id self, SEL _cmd, id note) {
	(void)self;
	(void)_cmd;
	(void)note;
	/* TODO: buggy
	gui_info *ui = nil;
	object_getInstanceVariable(self, "gui_info", (void *)&ui);
	NSRect frame = cocoa_frame(cocoa_send(note, "object"));
	uint32_t *new_buf = realloc(ui->buf, frame.size.width * frame.size.height * sizeof(uint32_t));
	if (!new_buf) return;

	ui->buf = new_buf;
	ui->width = frame.size.width;
	ui->height = frame.size.height;*/
}

static void gui_draw_rect(id v, SEL s, CGRect r) {
	(void)r, (void)s;
	gui_info *ui = nil;
	object_getInstanceVariable(v, "gui_info", (void *)&ui);
	CGContextRef context = (CGContextRef)cocoa_send(cocoa_get("NSGraphicsContext", "currentContext"), "graphicsPort");
	CGColorSpaceRef space = CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef provider = CGDataProviderCreateWithData(
		NULL, ui->buf, ui->width * ui->height * 4, NULL);
	CGImageRef img = CGImageCreate(ui->width, ui->height, 8, 32, ui->width * 4, space,
		kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, provider, NULL, false, kCGRenderingIntentDefault);
	CGColorSpaceRelease(space);
	CGDataProviderRelease(provider);
	CGContextDrawImage(context, CGRectMake(0, 0, ui->width, ui->height), img);
	CGImageRelease(img);
}

NSTextField cocoa_text_field(id gui, ui_field_type kind, char *label, char *field,
	float x, float y, float width, uintptr_t tag) {
	gui_info *ui = (gui_info *)gui;
	NSTextField text, slabel = nil;
	if (label) {
		slabel = cocoa_send(cocoa_send_rect(cocoa_alloc("NSTextField"), "initWithFrame:",
			x + 1, y + 15, width, 10), "autorelease");
		cocoa_set(slabel, "setBezeled:", NO);
		cocoa_set(slabel, "setEditable:", NO);
		cocoa_set(slabel, "setSelectable:", NO);
		cocoa_set(slabel, "setDrawsBackground:", NO);
		cocoa_set_with(slabel, "setFont:", (id)ui->font);
		cocoa_set_with(slabel, "setStringValue:", (id)cocoa_str(label));
		cocoa_postpairwith_func(cocoa_send(ui->wnd, "contentView"),
			sel_getUid("addSubview:positioned:relativeTo:"), slabel, NSWindowBelow, nil);
	}

	text = cocoa_send(cocoa_send_rect((kind == field_secret ? cocoa_alloc("NSSecureTextField") : cocoa_alloc("NSTextField")),
		"initWithFrame:", x, y - 5, width, 21), "autorelease");
	cocoa_set_with(text, "setDelegate:", (id)ui->delegate);
	cocoa_select(text, "becomeFirstResponder");
	cocoa_set_with(text, "setStringValue:", (id)cocoa_str(field));
	cocoa_postpairwith_func(cocoa_send(ui->wnd, "contentView"),
		sel_getUid("addSubview:positioned:relativeTo:"), text, NSWindowAbove, slabel);
		/* Setup `viewWithTag:` usage */
	cocoa_set(text, "setTag:", tag);
	return text;
}

FORCEINLINE NSButton cocoa_form_button(id window, char *title, char *action, float x, float y) {
	return cocoa_button(window, title, action, x, y, 80, NSTexturedSquareBezelStyle, false);
}

FORCEINLINE NSView cocoa_content_view(id window) {
	return (NSView)cocoa_send(window, "contentView");
}

FORCEINLINE void cocoa_check(id window, NSButton button, BOOL onOff) {
	if (onOff == YES)
		cocoa_set_with(button, "performClick:", window);
	cocoa_set(cocoa_send(window, "contentView"), "setNeedsDisplay:", YES);
}

static BOOL should_end_editing(__GUI_MENU__) {
	(void)selector;
	gui_info *ui = nil;
	object_getInstanceVariable(self, "gui_info", (void *)&ui);
	NSTextField field = (NSTextField)cocoa_send(cocoa_send(data, "object"), "representedObject");
	ui_field *form = (ui_field *)ui->app->app_array;
	BOOL is_ready = NO;
	int i;

	for (i = 0; i < ui->app->code; i++) {
		if (form[i].index == field)
			break;
	}

	switch (form[i].kind) {
		case field_number:
		case field_text:
			is_ready = is_MinLength(field, form[i]) && is_MaxLength(field, form[i]);
			break;
		case field_email:
			is_ready = is_EmailValid(field, form[i]);
			break;
		case field_secret:
			is_ready = is_PasswordValid(field, form[i]);
			break;
		case field_date:
			is_ready = YES;
			break;
		case field_regex:
		case field_url:
			is_ready = YES;
			break;
	}

	return is_ready;
}

ui_wnd_t gui_statusline(ui_wnd_t window, ui_wnd_t alignto, const char *message, int y, int width) {
	ui_wnd_t statusLine = cocoa_send(cocoa_send_rect(cocoa_alloc("NSTextField"), "initWithFrame:",
		6, y, width, 13), "autorelease");
	cocoa_set(statusLine, "setBezeled:", NO);
	cocoa_set(statusLine, "setEditable:", NO);
	cocoa_set(statusLine, "setSelectable:", NO);
	cocoa_set(statusLine, "setDrawsBackground:", YES);
	cocoa_set_with(statusLine, "setFont:", (id)cocoa_font("Arial", 9));
	cocoa_set_with(statusLine, "setStringValue:", (id)cocoa_str(message));
	cocoa_set(statusLine, "setAutoresizingMask:", NSViewMinXMargin | NSViewWidthSizable);
	cocoa_postpairwith_func(cocoa_send(window, "contentView"),
		sel_getUid("addSubview:positioned:relativeTo:"), statusLine, NSWindowOut, alignto);

	return statusLine;
}

int gui_form(gui_info *ui, const char *title, Form *fill, int numFields, ui_form_cb verify) {
	int i, y = 0, max_width = 0, spacing = 30;
	NSTextField text = nil, slabel = nil;

	/* calculate form width based off longest field width */
	for (i = 0; i < numFields; i++) {
		if (fill[i].width > max_width)
			max_width = fill[i].width;
	}

	ui->title = title;
	ui->width = max_width + 12;
	/* calculate form height based off number of text fields provided */
	ui->height = numFields * 45;
	if (!gui_window(ui, ui->title, ui->width, ui->height, -1))
		return 0;

	ui->user_data = verify;
	ui->app->code = numFields;
	ui->font = cocoa_font("Arial", 9);
	ui->txtCr = CGColorCreateGenericRGB(RGB_RED, 0);
	ui->bkCr = CGColorCreateGenericRGB(RGB_BLACK, 0);

	class_addMethod(ui->delegate, sel_registerName("verify_form:"), (IMP)verify_form, "c@:@");
	class_addMethod(ui->delegate, sel_registerName("cancel_form:"), (IMP)cancel_form, "c@:@");
	class_addMethod(ui->delegate, sel_registerName("reset_form:"), (IMP)reset_form, "c@:@");
	class_addMethod(ui->delegate, sel_registerName("textShouldEndEditing:"), (IMP)should_end_editing, "c@:@");
	cocoa_postnotification_func((id)cocoa_get("NSNotificationCenter", "defaultCenter"),
		sel_getUid("addObserver:selector:name:object:"), (id)ui->delegate_instance, sel_getUid("textShouldEndEditing:"),
		(id)cocoa_str("NSControlTextDidEndEditingNotification"), nil);

	for (i = 0; i < numFields; i++) {
		/* Setup spacing between each `textfield` with `caption/label` */
		y += spacing;
		text = cocoa_text_field((id)ui, fill[i].kind, fill[i].caption, fill[i].value, 5,
			ui->height - y, fill[i].width, fill[i].ID);
		/* Set what `textShouldEndEditing:` callback notification will receive */
		cocoa_set_with(text, "setRepresentedObject:", text);
		/* Store `NSTextField` into provided `Form` for `verify_form` and `should_end_editing`
		 validation process */
		fill[i].index = text;
	}

	NSButton button2, button1 = cocoa_form_button(ui->wnd, "Confirm", "verify_form:", 136, 25);
	button2 = cocoa_form_button(ui->wnd, "Cancel", "cancel_form:", 215, 25);

	/* Setup statusline area in form for `error` feedback */
	ui->statusLine = gui_statusline(ui->wnd, nil, "Fill out", 0, ui->width - 12);

	/* Store provided `Form` for `verify_form:` button click verification process */
	ui->app->app_array = (void **)fill;
	cocoa_set(ui->wnd, "setIsVisible:", YES);
	return 1;
}

void gui_file(__GUI_FILE__) {
	gui_info ui = {0};
	if (gui_window(&ui, cocoa_tochar(file), 600, 600, false)) {
		NSString data = (NSString)cocoa_send_with(cocoa_alloc("NSString"), "initWithContentsOfFile:", (id)file);

		id view = cocoa_send_rect(cocoa_alloc("View"), "initWithFrame:", 0, 0, 480, 480);
		cocoa_set_with(ui.wnd, "setContentView:", view);
		id cFrame = cocoa_send(cocoa_send(ui.wnd, "contentView"), "frame");

		NSScrollView scrollview = (NSScrollView)cocoa_send_with(cocoa_alloc("NSScrollView"), "initWithFrame:", cFrame);
		NSSize contentSize = cocoa_size_func(scrollview, sel_getUid("contentSize"));
		cocoa_set(scrollview, "setBorderType:", NSNoBorder);
		cocoa_set(scrollview, "setHasVerticalScroller:", YES);
		cocoa_set(scrollview, "setHasHorizontalScroller:", NO);
		cocoa_set(scrollview, "setAutoresizingMask:", NSViewWidthSizable | NSViewHeightSizable);

		/* create the NSTextView and add it to the window */
		NSTextView theTextView = cocoa_send_rect(cocoa_alloc("NSTextView"), "initWithFrame:",
			0, 0, contentSize.width, contentSize.height);

		cocoa_set_size(theTextView, "setMinSize:", 0.0, contentSize.height);
		cocoa_set_size(theTextView, "setMaxSize:", FLT_MAX, FLT_MAX);
		cocoa_set(theTextView, "setVerticallyResizable:", YES);
		cocoa_set(theTextView, "setHorizontallyResizable:", YES);
		cocoa_set(theTextView, "setAutoresizingMask:", (NSViewWidthSizable | NSViewHeightSizable));
		cocoa_set_size(cocoa_send(theTextView, "textContainer"), "setContainerSize:", FLT_MAX, FLT_MAX);
		cocoa_set(cocoa_send(theTextView, "textContainer"), "setWidthTracksTextView:", YES);

		cocoa_set_with(scrollview, "setDocumentView:", theTextView);
		cocoa_set_with(ui.wnd, "setContentView:", scrollview);
		cocoa_set_with(ui.wnd, "makeKeyAndOrderFront:", nil);
		cocoa_set_with(ui.wnd, "makeFirstResponder:", theTextView);

		cocoa_set_with(theTextView, "setString:", (id)data);
		cocoa_set(theTextView, "setEditable:", YES);
		cocoa_set(theTextView, "setSelectable:", YES);

		gui_active(ui);
		gui_destroy(ui);
	}
}

void gui_save_dialog(__GUI_MENU__) {
	(void)self;
	(void)selector;
	ui_file_cb ui_save_func = (ui_file_cb)cocoa_send(data, "representedObject");
	NSSavePanel saveFileDialog = cocoa_send(cocoa_init(cocoa_alloc("NSSavePanel")), "autorelease");

	cocoa_set(saveFileDialog, "setCanCreateDirectories:", YES);
	cocoa_set_with(saveFileDialog, "setAllowedFileTypes:", (id)cocoa_array((id)cocoa_str("*"), nil));
	cocoa_set_with(saveFileDialog, "setDirectoryURL:", cocoa_get_with("NSURL", "fileURLWithPath:",
		cocoa_send((id)NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES), "firstObject")));
	cocoa_set_with(saveFileDialog, "setNameFieldStringValue:", (id)cocoa_str("SaveFile.txt"));

	NSModalResponse response = cocoa_status(saveFileDialog, "runModal");
	if (response == NSModalResponseOK) {
		fprintf(stderr, "%s\n", cocoa_tochar(cocoa_sprintf("File = %@",
			cocoa_send(cocoa_send(saveFileDialog, "URL"), "path"))));
	}
}

void gui_open_dialog(__GUI_MENU__) {
	(void)selector;
	ui_file_cb ui_open_func = (ui_file_cb)cocoa_send(data, "representedObject");
	NSSavePanel openFileDialog = cocoa_send(cocoa_init(cocoa_alloc("NSOpenPanel")), "autorelease");

	cocoa_set(openFileDialog, "setCanChooseFiles:", YES);
	cocoa_set(openFileDialog, "setCanChooseDirectories:", NO);
	cocoa_set(openFileDialog, "setAllowsMultipleSelection:", NO);
	cocoa_set_with(openFileDialog, "setAllowedFileTypes:", (id)cocoa_array((id)cocoa_str("*"), nil));
	cocoa_set_with(openFileDialog, "setDirectoryURL:", cocoa_get_with("NSURL", "fileURLWithPath:",
		cocoa_send((id)NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES), "firstObject")));

	NSModalResponse response = cocoa_status(openFileDialog, "runModal");
	if (response == NSModalResponseOK) {
		NSString file = (NSString)cocoa_send(cocoa_array_index((NSArray)cocoa_send(openFileDialog, "URLs"), 0), "path");
		if (ui_open_func) {
			gui_info *ui = nil;
			object_getInstanceVariable(self, "gui_info", (void *)&ui);
			ui_open_func(ui->app, cocoa_tochar(file));
		} else {
			gui_file(self, file);
		}
	}
}

int gui_menufont(gui_info *ui, const char *font) {
	ui->bar_info->font_info = cocoa_font(font, 12);
	ui->bar_info->attr = cocoa_dict(NSFontAttributeName, (id)ui->bar_info->font_info);
	return (ui->bar_info->font_info == nil || ui->bar_info->attr == nil) ? 0 : 1;
}

int gui_menubar(gui_info *ui, int numof_menus) {
	if ((ui->bar_info = (menu_bar_t *)calloc(1, sizeof(menu_bar_t)))) {
		ui->bar_info->hMenubar = cocoa_send(NSApp, "mainMenu");
		ui->bar_info->num_menus = numof_menus;
		ui->bar_info->state = 0;
		ui->bar_info->menus = (menu_t *)calloc(1, GetNumMenus(ui->bar_info) * sizeof(menu_t));
		return 1;
	}

	return 0;
}

int gui_menu(gui_info *ui, int num_menu, menuitem_t *items, int number_items, int menu_id, char *name) {
	int r = 0;
	NSMenuItem item = nil;
	menu_t *menu = &(ui->bar_info->menus[num_menu]);
	menu->num_items = number_items;
	menu->selected = none_selected;
	menu->menu_id = menu_id;
	menu->items = items;
	menu->hMenu = cocoa_send_with(cocoa_autorelease("NSMenu"), "initWithTitle:", (id)cocoa_str(name));
	if (menu->hMenu) {
		id appMenuItem = cocoa_send(cocoa_new("NSMenuItem"), "autorelease");
		cocoa_set_with(ui->bar_info->hMenubar, "addItem:", appMenuItem);
		for (r = 0; r < number_items; r++) {
			// add menu items
			if (items[r].item_name == nil && items[r].action == nil) {
				cocoa_menu_separator(menu->hMenu);
			} else {
				item = cocoa_menuitem_action(menu->hMenu, nil, items[r].item_name,
					items[r].action, items[r].alphaKey, items[r].data);
				cocoa_set(item, "setTag:", (NSInteger)items[r].menu_id);
				cocoa_menuitem_font(item, ui->bar_info->attr);
			}
		}

		// attach menu bar to the window
		cocoa_set_with(appMenuItem, "setSubmenu:", menu->hMenu);
	}

	return r;
}

int gui_message_box(id app, const char *title, const char *text, const Button *buttons, int numButtons) {
	int i;
	//size_t sz = strlen(text) + 200;
	NSAlert alert = cocoa_send(cocoa_init(cocoa_alloc("NSAlert")), "autorelease");
	//NSView accessoryView = cocoa_send(cocoa_send_rect(cocoa_alloc("NSView"), "initWithFrame:", 0, 0, sz, 0), "autorelease");
	//cocoa_set_with(alert, "setAccessoryView:", (id)accessoryView);
	cocoa_set_with(alert, "setMessageText:", (id)cocoa_str(title));
	cocoa_set_with(alert, "setInformativeText:", (id)cocoa_str(text));
	cocoa_set(alert, "setAlertStyle:", (numButtons == 1	? NSAlertStyleInformational
		: (numButtons == 2 ? NSAlertStyleWarning : NSAlertStyleCritical)));
	for (i = 0; i < numButtons; i++)
		cocoa_set_with(alert, "addButtonWithTitle:", (id)cocoa_str(buttons[i].label));

	//cocoa_select(alert, "layout");
	return ((int)cocoa_status(alert, "runModal") - NSAlertFirstButtonReturn) + 1;
}

int gui_window(gui_info *ui, const char *title, int width, int height, int buffered) {
	ui->title = title;
	ui->width = (int)width;
	ui->height = (int)height;
	if (main_gui_info == NULL) {
		main_gui_info = ui;
		cocoa_application(ui);
		if (ui->wnd == NULL) {
			fprintf(stderr, "Failed to initialized NSApplication...  terminating...\n");
			return 0;
		}

		Class windelegate = objc_allocateClassPair(objc_getClass("NSObject"), "GuiDelegate", 0);
		class_addProtocol(windelegate, objc_getProtocol("NSWindowDelegate"));
		class_replaceMethod(windelegate, sel_getUid("windowWillClose:"),
			(IMP)webView_windowWillClose, "v@:@");
		class_addMethod(windelegate, sel_getUid("windowShouldClose:"), (IMP)should_close, 0);
		class_addProtocol(windelegate, objc_getProtocol("WKNavigationDelegate"));
		class_addProtocol(windelegate, objc_getProtocol("WKUIDelegate"));
		class_addMethod(windelegate, sel_getUid("userContentController:didReceiveScriptMessage:"),
			(IMP)webView_didReceiveScriptMessage, "v@:@@");
		class_addMethod(windelegate,
			sel_getUid("webView:didFinishNavigation:"),
			(IMP)webView_didFinishNavigation, "i@:@");
		class_addMethod(windelegate,
			sel_getUid("webView:runOpenPanelWithParameters:"
				"initiatedByFrame:completionHandler:"),
			(IMP)run_open_panel, "v@:@@@?");
		class_addMethod(windelegate,
			sel_getUid("webView:runJavaScriptAlertPanelWithMessage:"
				"initiatedByFrame:completionHandler:"),
			(IMP)run_alert_panel, "v@:@@@?");
		class_addMethod(windelegate,
			sel_getUid("webView:runJavaScriptConfirmPanelWithMessage:"
				"initiatedByFrame:completionHandler:"),
			(IMP)run_confirmation_panel, "v@:@@@?");
		class_addMethod(windelegate,
			sel_getUid("webView:decidePolicyForNavigationResponse:decisionHandler:"),
			(IMP)make_nav_policy_decision, "v@:@@?");

		class_addIvar(windelegate, "gui_info", sizeof(gui_info *), rint(log2(sizeof(gui_info *))), "L");
		objc_registerClassPair(windelegate);

		id v = cocoa_init(cocoa_alloc_class(windelegate));
		cocoa_set_with(ui->wnd, "setDelegate:", v);
		object_setInstanceVariable(v, "gui_info", ui);

		cocoa_set(ui->wnd, "setAutorecalculatesKeyViewLoop:", YES);
		ui->delegate = windelegate;
		ui->app->wnd = ui->wnd;
		ui->app->gui = ui;
	} else {
		if (buffered == YES) {
			ui->buf = malloc(ui->width * ui->height * sizeof(uint32_t));
			if (!ui->buf)
				return 0;
		}

		ui->pool = cocoa_new("NSAutoreleasePool");
		int mask = (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable);
		ui->wnd = cocoa_window(ui->width, (main_gui_info->height - ui->height), ui->width, ui->height,
			(buffered == -1 ? mask & ~NSWindowStyleMaskResizable : mask), NSBackingStoreBuffered, NO);

		Class c = objc_allocateClassPair((Class)objc_getClass("NSView"), "GuiView", 0);
		class_addMethod(c, sel_getUid("windowShouldClose:"), (IMP)should_close, "c@:@");
		if (buffered)
			class_addMethod(c, sel_getUid("drawRect:"), (IMP)gui_draw_rect, "i@:@@");

		class_addIvar(c, "gui_info", sizeof(gui_info *), rint(log2(sizeof(gui_info *))), "L");
		objc_registerClassPair(c);

		id v = cocoa_init(cocoa_alloc_class(c));
		cocoa_set_with(ui->wnd, "setDelegate:", v);
		cocoa_set_with(ui->wnd, "setContentView:", v);
		cocoa_select(ui->wnd, "becomeFirstResponder");
		cocoa_set_with(ui->wnd, "setTitle:", (id)cocoa_str(ui->title));
		cocoa_set_with(ui->wnd, "makeKeyAndOrderFront:", nil);
		object_setInstanceVariable(v, "gui_info", ui);

		cocoa_set(ui->wnd, "setAutorecalculatesKeyViewLoop:", YES);
		ui->delegate = c;
		ui->delegate_instance = v;
		ui->app->wnd = ui->wnd;
		ui->app->gui = ui;
		ui->app->running = YES;
		if (buffered == YES) {
			class_addMethod(c, sel_getUid("windowDidResize:"), (IMP)gui_window_resize, "v@:@");
			cocoa_postnotification_func((id)cocoa_get("NSNotificationCenter", "defaultCenter"),
				sel_getUid("addObserver:selector:name:object:"), (id)v, sel_getUid("windowDidResize:"),
				(id)NSWindowDidResizeNotification, ui->wnd);
		}
	}

	return (ui->wnd != nil && NSApp != nil && AppDelClass != nil);
}

FORCEINLINE void gui_close(gui_info *ui) {
	cocoa_select(ui->wnd, "close");
	if (ui->bar_info && ui->pool)
		cocoa_select(ui->pool, "drain");

	ui->pool = nil;
	objc_disposeClassPair(ui->delegate);
	if (ui->bar_info) {
		free(ui->bar_info->menus);
		free(ui->bar_info);
		ui->bar_info = NULL;
	}

	if (ui->buf) {
		free(ui->buf);
		ui->buf = NULL;
	}
}

FORCEINLINE int gui_handler(gui_info *ui) {
	ui->app->running = YES;
	cocoa_select(NSApp, "finishLaunching");
	ui->pool = cocoa_new("NSAutoreleasePool");
	cocoa_select(cocoa_send(ui->wnd, "autorelease"), "makeMainWindow");
	while (ui->app->running) {
		NSEvent event = cocoa_next_event(NSApp, NSUIntegerMax, nil, NSDefaultRunLoopMode, YES);
		if (event) {
			cocoa_set_with(NSApp, "sendEvent:", event);
			cocoa_select(NSApp, "updateWindows");
		}

		if ((!main_gui_info->bar_info && main_gui_info->web->priv.should_exit))
			break;
	}

	return 0;
}

FORCEINLINE void gui_active(gui_info ui) {
	id ev = nil;
	ui_t *app = (ui_t *)ui.app;
	cocoa_set(cocoa_send(app->wnd, "contentView"), "setNeedsDisplay:", YES);
	while (app->running) {
		cocoa_set_with(NSApp, "sendEvent:",
			cocoa_next_event(NSApp, NSUIntegerMax, nil, NSEventTrackingRunLoopMode, YES));
		cocoa_select(NSApp, "updateWindows");
	}
}

FORCEINLINE void gui_destroy(gui_info ui) {
	gui_close(&ui);
}

// clang-format off
static const uint8_t _GUI_KEYCODES[128] = {65,83,68,70,72,71,90,88,67,86,0,66,81,87,69,82,89,84,49,50,51,52,54,53,61,57,55,45,56,48,93,79,85,91,73,80,10,76,74,39,75,59,92,44,47,78,77,46,9,32,96,8,0,27,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,2,3,127,0,5,0,4,0,20,19,18,17,0};
// clang-format on
int gui_loop(gui_info *ui) {
	cocoa_set(cocoa_send(ui->wnd, "contentView"), "setNeedsDisplay:", YES);
	id ev = cocoa_next_event(NSApp, NSUIntegerMax, nil, NSDefaultRunLoopMode, YES);
	NSUInteger evtype = (NSUInteger)cocoa_send(ev, "type");
	switch (evtype) {
		case NSEventTypeLeftMouseDown: /* NSEventTypeMouseDown */
			ui->mouse |= 1;
			break;
		case NSEventTypeLeftMouseUp: /* NSEventTypeMouseUp*/
			ui->mouse &= ~1;
			break;
		case NSEventTypeMouseMoved:
		case NSEventTypeLeftMouseDragged: { /* NSEventTypeMouseMoved */
				CGPoint xy = *(CGPoint *)cocoa_send(ev, "locationInWindow");
				ui->x = (int)xy.x;
				ui->y = (int)(ui->height - xy.y);
				return 0;
			}
		case NSEventTypeKeyDown: /*NSEventTypeKeyDown*/
		case NSEventTypeKeyUp: /*NSEventTypeKeyUp:*/ {
				NSUInteger k = (NSUInteger)cocoa_send(ev, "keyCode");
				ui->keys[k < 127 ? _GUI_KEYCODES[k] : 0] = evtype == NSEventTypeKeyDown;
				NSUInteger mod = (NSUInteger)cocoa_send(ev, "modifierFlags") >> 17;
				ui->mod = (mod & 0xc) | ((mod & 1) << 1) | ((mod >> 1) & 1);
				if (evtype == NSEventTypeKeyDown)
					cocoa_set_with(NSApp, "sendEvent:", ev);
				return 0;
			}
	}

	cocoa_set_with(NSApp, "sendEvent:", ev);

	return ui->app->running == YES ? 0 : 1;
}
#elif defined(_WIN32)
#define ID_WINDOW_ICON	900

// clang-format off
static const uint8_t _GUI_KEYCODES[] = {0,27,49,50,51,52,53,54,55,56,57,48,45,61,8,9,81,87,69,82,84,89,85,73,79,80,91,93,10,0,65,83,68,70,71,72,74,75,76,59,39,96,0,92,90,88,67,86,66,78,77,44,46,47,0,0,0,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,17,3,0,20,0,19,0,5,18,4,26,127};
// clang-format on
typedef struct BINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[3];
}BINFO;

static LRESULT CALLBACK gui_wndproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	gui_info *ui = (gui_info *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	static HMENU hmenu;             // handle to main menu
	static COLORREF crSelText;  // text color of selected item
	static COLORREF crSelBkgnd = RGB(173, 216, 230); // background color of selected item
	COLORREF crText;            // text color of unselected item
	COLORREF crBkgnd;           // background color unselected item
	LPMEASUREITEMSTRUCT lpmis;  // pointer to item of data
	LPDRAWITEMSTRUCT lpdis;     // pointer to item drawing data
	HDC hdc;                    // handle to screen DC
	SIZE size;                  // menu-item text extents
	WORD wCheckX;               // check-mark width
	int nTextX;                 // width of menu item
	int nTextY;                 // height of menu item
	int i;                      // loop counter
	HFONT hfontOld;             // handle to old font
	BOOL fSelected = FALSE;     // menu-item selection flag
	size_t length = 0;
	size_t *pcch = &length;
	HRESULT hResult;
	HMENU hCharacterMenu;
	menuitem_t *pmyitem;
	switch (msg) {
		case WM_ERASEBKGND:
			{
				HDC hdc = (HDC)wParam;
				RECT rc;
				GetClientRect(hwnd, &rc);
				HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255)); // white background
				FillRect(hdc, &rc, brush);
				DeleteObject(brush);
				return true; // background erased
			}
		case WM_MEASUREITEM:
			// Retrieve a device context for the main window.
			hdc = GetDC(hwnd);

			// Retrieve pointers to the menu item's
			// MEASUREITEMSTRUCT structure and MYITEM structure.
			lpmis = (LPMEASUREITEMSTRUCT)lParam;
			pmyitem = (menuitem_t *)lpmis->itemData;

			// Select the font associated with the item into
			// the main window's device context.
			hfontOld = (HFONT)SelectObject(hdc, pmyitem->hfont);

			// Retrieve the width and height of the item's string,
			// and then copy the width and height into the
			// MEASUREITEMSTRUCT structure's itemWidth and
			// itemHeight members.
			hResult = StringCchLength(pmyitem->item_name, STRSAFE_MAX_CCH, pcch);
			if (FAILED(hResult)) {
				// Add code to fail as securely as possible.
				return (LRESULT)0;
			}

			GetTextExtentPoint32(hdc, pmyitem->item_name, *pcch, &size);
			lpmis->itemWidth = size.cx;
			lpmis->itemHeight = size.cy;

			// Select the old font back into the device context,
			// and then release the device context.
			SelectObject(hdc, hfontOld);
			ReleaseDC(hwnd, hdc);
			return TRUE;
		case WM_DRAWITEM:
			// Get pointers to the menu item's DRAWITEMSTRUCT
			// structure and MYITEM structure.
			lpdis = (LPDRAWITEMSTRUCT)lParam;
			pmyitem = (menuitem_t *)lpdis->itemData;

			// If the user has selected the item, use the selected
			// text and background colors to display the item.
			if (lpdis->itemState & ODS_SELECTED) {
				crText = SetTextColor(lpdis->hDC, crSelText);
				crBkgnd = SetBkColor(lpdis->hDC, crSelBkgnd);
				fSelected = TRUE;
			}

			// Remember to leave space in the menu item for the
			// check-mark bitmap. Retrieve the width of the bitmap
			// and add it to the width of the menu item.
			wCheckX = GetSystemMetrics(SM_CXMENUCHECK);
			nTextX = wCheckX + lpdis->rcItem.left;
			nTextY = lpdis->rcItem.top;

			// Select the font associated with the item into the
			// item's device context, and then draw the string.
			hfontOld = (HFONT)SelectObject(lpdis->hDC, pmyitem->hfont);
			hResult = StringCchLength(pmyitem->item_name, STRSAFE_MAX_CCH, pcch);
			if (FAILED(hResult)) {
				// Add code to fail as securely as possible.
				return (LRESULT)0;
			}

			ExtTextOut(lpdis->hDC, nTextX, nTextY, ETO_OPAQUE, &lpdis->rcItem, pmyitem->item_name, *pcch, NULL);

			// Select the previous font back into the device
			// context.
			SelectObject(lpdis->hDC, hfontOld);

			// Return the text and background colors to their
			// normal state (not selected).
			if (fSelected) {
				SetTextColor(lpdis->hDC, crText);
				SetBkColor(lpdis->hDC, crBkgnd);
			}

			return TRUE;
		case WM_CLOSE:
			if (ui == main_gui_info)
				main_gui_shutdown = true;
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_COMMAND:
			menuitem_t *menus_active;
			int i, x, count, which = LOWORD(wParam);
			if (ui->bar_info) {
				for (i = 0; i < ui->bar_info->num_menus; i++) {
					count = ui->bar_info->menus[i].num_items;
					menus_active = ui->bar_info->menus[i].items;
					for (x = 0; x < count; x++) {
						if (which == menus_active[x].menu_id) {
							ui_t *app = ui->app;
							app->wnd = ui->wnd;
							app->app_data = ui->hinst;
							app->gui = ui;
							menus_active[x].action(app, menus_active[x].data);
							return 0;
						}
					}

					if (!ui->bar_info)
						break;
				}
			}
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

static LRESULT CALLBACK gui_wndproc_arcade(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	gui_info *ui = (gui_info *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch (msg) {
		case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);
				HDC memdc = CreateCompatibleDC(hdc);
				HBITMAP hbmp = CreateCompatibleBitmap(hdc, ui->width, ui->height);
				HBITMAP oldbmp = SelectObject(memdc, hbmp);
				BINFO bi = {{sizeof(bi), ui->width, -ui->height, 1, 32, BI_BITFIELDS}};
				bi.bmiColors[0].rgbRed = 0xff;
				bi.bmiColors[1].rgbGreen = 0xff;
				bi.bmiColors[2].rgbBlue = 0xff;
				SetDIBitsToDevice(memdc, 0, 0, ui->width, ui->height, 0, 0, 0, ui->height,
					ui->buf, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
				BitBlt(hdc, 0, 0, ui->width, ui->height, memdc, 0, 0, SRCCOPY);
				SelectObject(memdc, oldbmp);
				DeleteObject(hbmp);
				DeleteDC(memdc);
				EndPaint(hwnd, &ps);
			} break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			ui->mouse = (msg == WM_LBUTTONDOWN);
			break;
		case WM_MOUSEMOVE:
			ui->y = HIWORD(lParam), ui->x = LOWORD(lParam);
			break;
		case WM_KEYDOWN:
		case WM_KEYUP: {
				ui->mod = ((GetKeyState(VK_CONTROL) & 0x8000) >> 15) |
					((GetKeyState(VK_SHIFT) & 0x8000) >> 14) |
					((GetKeyState(VK_MENU) & 0x8000) >> 13) |
					(((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) >> 12);
				ui->keys[_GUI_KEYCODES[HIWORD(lParam) & 0x1ff]] = !((lParam >> 31) & 1);
			} break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void gui_file(__GUI_FILE__) {
	HANDLE hFile;
	hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD dwFileSize;

		dwFileSize = GetFileSize(hFile, NULL);
		if (dwFileSize != 0xFFFFFFFF) {
			LPSTR pszFileText;

			pszFileText = GlobalAlloc(GPTR, dwFileSize + 1);
			if (pszFileText != NULL) {
				DWORD dwRead;
				if (ReadFile(hFile, pszFileText, dwFileSize, &dwRead, NULL)) {
					pszFileText[dwFileSize] = 0; // Add null terminator
					HWND hEdit = CreateWindowExA(WS_EX_RIGHTSCROLLBAR, TEXT("edit"), file,
						WS_VISIBLE | WS_POPUPWINDOW | WS_CHILD | WS_SIZEBOX |
						WS_OVERLAPPEDWINDOW | WS_VSCROLL | ES_MULTILINE,
						150, 150, 565, 320, NULL, NULL, NULL, NULL);
					SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
					if (SetWindowText(hEdit, pszFileText)) {
						self->app_array = (void **)hEdit;
					}
				}
				GlobalFree(pszFileText);
			}
		}
		CloseHandle(hFile);
	}
}

BOOL gui_save(__GUI_FILE__) {
	HANDLE hFile;
	BOOL bSuccess = FALSE;

	hFile = CreateFile(file, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD dwTextLength;

		dwTextLength = GetWindowTextLength((HWND)self->app_array);
		// No need to bother if there's no text.
		if (dwTextLength > 0) {
			LPSTR pszText;
			DWORD dwBufferSize = dwTextLength + 1;

			pszText = GlobalAlloc(GPTR, dwBufferSize);
			if (pszText != NULL) {
				if (GetWindowText((HWND)self->app_array, pszText, dwBufferSize)) {
					DWORD dwWritten;

					if (WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL))
						bSuccess = TRUE;
				}
				GlobalFree(pszText);
			}
		}
		CloseHandle(hFile);
	}
	return bSuccess;
}

void gui_save_dialog(__GUI_MENU__) {
	int ok;
	OPENFILENAME ofn;
	static char result_buf[2048];
	result_buf[0] = '\0';

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.hwndOwner = self->wnd ? self->wnd : NULL;
	ofn.hInstance = self->app_data ? self->app_data : NULL;
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = TEXT("All files(*.*)\0*.*\0");//(opt);
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = result_buf;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.nMaxFile = sizeof(result_buf) - 1;
	ofn.lpstrInitialDir = "./";
	ofn.lpstrTitle = "Select File";
	ofn.lpstrDefExt = "*.*";
	ok = GetSaveFileName(&ofn);
	if (ok && !data) {
		gui_save(self, ofn.lpstrFile);
	} else if (ok && data) {
		((ui_file_cb)data)(self, ofn.lpstrFile);
	}
}

void gui_open_dialog(__GUI_MENU__) {
	int ok;
	OPENFILENAME ofn;
	static char result_buf[2048];
	result_buf[0] = '\0';

	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.hwndOwner = self->wnd ? self->wnd : NULL;
	ofn.hInstance = self->app_data ? self->app_data : NULL;
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFilter = TEXT("All files(*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = result_buf;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.nMaxFile = sizeof(result_buf) - 1;
	ofn.lpstrInitialDir = "./";
	ofn.lpstrTitle = "Select File";
	ofn.lpstrDefExt = "*.*";
	ok = GetOpenFileName(&ofn);
	if (ok && !data) {
		gui_file(self, ofn.lpstrFile);
	} else if (ok && data) {
		((ui_file_cb)data)(self, ofn.lpstrFile);
	}
}

static LRESULT CALLBACK gui_wndproc_form(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	gui_info *ui = (gui_info *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (ui == NULL)
		return DefWindowProc(hwnd, msg, wParam, lParam);

	ui_form_cb verify = ui->user_data;
	const ui_t *app = ui->app;
	ui_field form, *fill = (ui_field *)ui->app->app_array;
	ui_form_t field;
	const unsigned long numFields = app->code;
	int i, action, which;
	HWND hStatus, hEdit, hReady;
	HDC hdc;

	switch (msg) {
		case WM_CTLCOLOREDIT:
			if (ui == main_gui_info)
				return 0;

			if (lParam == (LRESULT)GetFocus()) {
				hdc = (HDC)wParam;
				SetTextColor(hdc, RGB(0, 0, 0));
				SetBkColor(hdc, RGB(255, 255, 255));
				return (LRESULT)GetStockObject(WHITE_BRUSH);
			} else if (ui->app->extra != 0) {
				hdc = (HDC)wParam;
				SetTextColor(hdc, RGB(128, 128, 128));
				SetBkColor(hdc, RGB(0, 255, 0));
				return (LRESULT)GetStockObject(WHITE_BRUSH);
			} else {
				hdc = (HDC)wParam;
				SetTextColor(hdc, RGB(255, 255, 255)); 	// RGB_WHITE
				SetBkColor(hdc, RGB(255, 0, 0)); 		// RGB_RED
				return (LRESULT)GetStockObject(WHITE_BRUSH);
			}
		case WM_DRAWITEM:
			LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
			PTSTR ptStr = (PTSTR)lpDIS->itemData;
			SetTextColor(lpDIS->hDC, RGB(0xFF, 00, 00));
			ExtTextOut(lpDIS->hDC, 0, 0, 0, &lpDIS->rcItem, ptStr, _tcslen(ptStr), NULL);
			return (LRESULT)0;
		case WM_COMMAND:
			which = LOWORD(wParam);
			action = HIWORD(wParam);
			if (action == BN_CLICKED && which == ID_GUI_CANCEL) {
				DestroyWindow(hwnd);
				return (LRESULT)0;
			} else if (action == BN_CLICKED && which == ID_GUI_CONFIRM) {
				hStatus = GetDlgItem(hwnd, ID_GUI_STATUS);
				for (i = 0; i < numFields; i++) {
					form = fill[i];
					which = form.ID;
					field = (ui_form_t)form.index;
					char error[100] = {0};
					hReady = GetDlgItem(hwnd, (ID_GUI_ERROR + which));
					if (str_field_valid(field, form)) {
						ui->app->extra = which;
						SendMessage(hReady, BM_SETCHECK, BST_CHECKED, 0);
						SendMessage(form.index, BST_FOCUS, SBT_OWNERDRAW, (LPARAM)form.value);
					} else {
						switch (form.kind) {
							case field_number:
								snprintf(error, sizeof(error), "Error: number must be between %d or %d digits",
									form.min, form.max);
								break;
							case field_text:
								snprintf(error, sizeof(error), "Error: length overflow %d or underflow %d",
									form.max, form.min);
								break;
							case field_secret:
								snprintf(error, sizeof(error), "Error: secret aleast 1 cap, 1 num and minimum %d",
									form.min);
								break;
							case field_email:
								snprintf(error, sizeof(error), "Error: invalid Email");
								break;
						}

						ui->app->extra = which;
						GetWindowTextA(field, form.value, form.max);
						SendMessage(hReady, BM_SETCHECK, BST_UNCHECKED, 0);
						SendMessage(field, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)form.value);
						SendMessage(hStatus, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)error);
						return (LRESULT)0;
					}

					ui->app->extra = which;
					GetWindowTextA(field, form.value, form.max);
					SendMessage(hReady, BM_SETCHECK, BST_CHECKED, 0);
					SendMessage(field, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)form.value);
					SendMessage(hStatus, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)"");
					if (verify) {
						CHAR status[260];
						if (verify(app, which, form.value, status)) {
						} else {
							SendMessage(hReady, BM_SETCHECK, BST_UNCHECKED, 0);
							hEdit = GetDlgItem(hwnd, ID_GUI_STATUS);
							SendMessage(hEdit, SB_SETTEXT, SBT_OWNERDRAW, (LPARAM)status);
							return (LRESULT)0;
						}
					}
				}
				DestroyWindow(hwnd);
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND windows_field(ui_wnd_t window, ui_wnd_t alignto, char *area, float x, float y, float width,
	ui_field_type kind, uintptr_t tag) {
	int estyle = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_LEFT;
	HWND hEdit = CreateWindow("Edit", area, (kind == field_secret ? estyle | ES_PASSWORD : estyle), x, y,
		width, 20, window, (HMENU)(tag), (HINSTANCE)alignto, NULL);
	SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEVICE_DEFAULT_FONT), MAKELPARAM(FALSE, 0));
	return hEdit;
}

HWND windows_text_field(gui_info *ui, ui_field_type kind, char *label, char *field,
	float x, float y, float width, uintptr_t tag) {
	if (label) {
		HWND hEdit = CreateWindow("Static", label, WS_CHILD | WS_VISIBLE | SS_LEFT, x + 1, y - 24,
			width, 11, ui->wnd, (HMENU)ID_GUI_STATIC, NULL, NULL);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	}

	return windows_field(ui->wnd, NULL, field, x, (y - 10), width, kind, tag);
}

HWND windows_button(ui_wnd_t window, char *label, intptr_t action, float x, float y, float width) {
	HWND button = CreateWindow("Button", label, WS_VISIBLE | WS_CHILD | BS_PUSHLIKE | WS_TABSTOP,
		x, y, width, 20, window, (HMENU)(action), NULL, NULL);
	SendMessage(button, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	return button;
}

FORCEINLINE HWND windows_form_button(gui_info *ui, char *title, intptr_t action, float x, float y) {
	return windows_button(ui->wnd, title, action, x, y, 80);
}

FORCEINLINE ui_wnd_t gui_statusline(ui_wnd_t window, ui_wnd_t alignto, const char *message, int y, int width) {
	ui_wnd_t hEdit = CreateWindow(STATUSCLASSNAME, message, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		6, y, width, 13, window, (HMENU)ID_GUI_STATUS, (HINSTANCE)alignto, NULL);
	SendMessage(hEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	return hEdit;
}

int gui_form(gui_info *ui, const char *title, Form *fill, int numFields, ui_form_cb verify) {
	int i, max_width = 0, y = 0, spacing = 35;
	ui_t *app = ui->app;
	HWND hEdit, pWnd = app->wnd;

	ui->hinst = app->app_data;
	ui->title = title;

	/* calculate form width based off longest field width */
	for (i = 0; i < numFields; i++) {
		if (fill[i].width > max_width)
			max_width = fill[i].width;
	}

	ui->width = max_width + 28;
	/* calculate form height based off number of text fields provided */
	ui->height = numFields * 65;
	ui->buf = NULL;
	ui->txtCr = RGB(255, 0, 0);
	ui->bkCr = RGB(0, 0, 0);
	ui->user_data = verify;
	ui->app->extra = 0;
	ui->app->app_data = NULL;
	ui->app->code = numFields;
	ui->app->gui = ui;

	memset(&ui->wc, 0, sizeof(ui->wc));
	ui->wc.cbSize = sizeof(WNDCLASSEX);
	ui->wc.style = CS_VREDRAW | CS_HREDRAW | CS_NOCLOSE;
	ui->wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	ui->wc.lpfnWndProc = gui_wndproc_form;
	ui->wc.hInstance = ui->hinst;
	ui->wc.hCursor = LoadCursor(0, IDC_ARROW);
	ui->wc.lpszClassName = "Edit control";
	RegisterClassEx(&ui->wc);

	if ((ui->wnd = CreateWindowEx((WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_DLGMODALFRAME) & ~WS_MAXIMIZE,
		ui->wc.lpszClassName, ui->title,
		(WS_CAPTION | WS_POPUP | WS_VISIBLE | WS_CHILD), 200, 200,
		ui->width, ui->height, pWnd, NULL, ui->hinst, NULL)) == NULL)
		return 0;

	SetWindowLongPtr(ui->wnd, GWLP_USERDATA, (LONG_PTR)ui);
	for (i = 0; i < numFields; i++) {
		y += spacing;
		hEdit = windows_text_field(ui, fill[i].kind, fill[i].caption, fill[i].value,
			5, y, fill[i].width, fill[i].ID);
		fill[i].index = hEdit;

		CreateWindow("Button", "", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_TEXT | BS_FLAT | BS_VCENTER, fill[i].width - 12, (y - 6), 12, 12, ui->wnd, (HMENU)(ID_GUI_ERROR + fill[i].ID), NULL, NULL);
	}

	windows_form_button(ui, "Confirm", ID_GUI_CONFIRM, 130, (ui->height - 83));
	windows_form_button(ui, "Cancel", ID_GUI_CANCEL, 215, (ui->height - 83));

	/* Setup statusline area in form for `error` feedback*/
	hEdit = gui_statusline(ui->wnd, NULL, "Fill out", ui->height, ui->width);
	ui->app->app_array = (void **)fill;
	ShowWindow(ui->wnd, SW_NORMAL);
	UpdateWindow(ui->wnd);
	return 1;
}

int gui_window(gui_info *ui, const char *title, int width, int height, int buffered) {
	if (main_gui_info == NULL) {
		main_gui_info = ui;
	}

	ui->title = title;
	ui->width = (int)width;
	ui->height = (int)height;
	if (buffered == true) {
		ui->buf = malloc(ui->width * ui->height * sizeof(uint32_t));
		if (!ui->buf)
			return 0;
	}

	ui->hinst = GetModuleHandle(NULL);
	memset(&ui->wc, 0, sizeof(ui->wc));
	ui->wc.cbSize = sizeof(WNDCLASSEX);
	ui->wc.style = CS_VREDRAW | CS_HREDRAW;
	ui->wc.lpfnWndProc = (!buffered ? gui_wndproc : gui_wndproc_arcade);
	ui->wc.hInstance = ui->hinst;
	ui->wc.hIcon = LoadIcon(ui->hinst, MAKEINTRESOURCE(ID_WINDOW_ICON));
	ui->wc.hCursor = LoadCursor(0, IDC_ARROW);
	ui->wc.lpszClassName = ui->title;
	RegisterClassEx(&ui->wc);

	if ((ui->wnd = CreateWindowEx((!buffered ? WS_EX_WINDOWEDGE : (WS_EX_CLIENTEDGE | WS_EX_TOPMOST)),
		ui->title, ui->title, (buffered ? WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX : WS_OVERLAPPEDWINDOW), CW_USEDEFAULT, CW_USEDEFAULT,
		ui->width, ui->height, NULL, NULL, ui->hinst, NULL)) == NULL)
		return 0;

	SetWindowLongPtr(ui->wnd, GWLP_USERDATA, (LONG_PTR)ui);
	ShowWindow(ui->wnd, SW_NORMAL);
	UpdateWindow(ui->wnd);
	return 1;
}

#include "webview-win32.c"

int gui_menu(gui_info *ui, int num_menu, menuitem_t *items, int number_items, int menu_id, char *name) {
	int r = 0;
	menu_t *menu = &(ui->bar_info->menus[num_menu]);
	menu->num_items = number_items;
	menu->selected = none_selected;
	menu->menu_id = menu_id;
	menu->items = items;
	menu->hMenu = CreateMenu();
	if (menu->hMenu) {
		for (r = 0; r < number_items; r++) {
			// add menu items
			if (items[r].item_name == NULL && items[r].action == NULL) {
				AppendMenu(menu->hMenu, MF_SEPARATOR, 0, NULL);
			} else {
				if (items[r].alphaKey != NULL) {
					char alphaKey[65] = {0};
					snprintf(alphaKey, sizeof(alphaKey), "%s\tCtrl+%s", items[r].item_name, items[r].alphaKey);
					AppendMenu(menu->hMenu, MF_STRING, items[r].menu_id, alphaKey);
					RegisterHotKey(ui->wnd, items[r].menu_id, MOD_CONTROL, items[r].alphaKey[0]);
				} else {
					AppendMenu(menu->hMenu, MF_STRING, items[r].menu_id, items[r].item_name);
				}
			}
		}
		AppendMenu(ui->bar_info->hMenubar, MF_POPUP, (UINT_PTR)menu->hMenu, name);

		// attach menu bar to the window
		SetMenu(ui->wnd, ui->bar_info->hMenubar);
	}

	return r;
}

int gui_menubar(gui_info *ui, int numof_menus) {
	if ((ui->bar_info = (menu_bar_t *)calloc(1, sizeof(menu_bar_t)))) {
		ui->bar_info->hMenubar = CreateMenu();
		ui->bar_info->num_menus = numof_menus;
		ui->bar_info->state = 0;
		ui->bar_info->menus = (menu_t *)calloc(1, GetNumMenus(ui->bar_info) * sizeof(menu_t));
		return 1;
	}

	return 0;
}

static int LoadStringEx(HMODULE hModule, UINT wID, PWSTR pBuffer, int cchBufferMax, WORD wLangId) {
	HRSRC hRsrc;
	int cch = 0;
	uintptr_t GroupId = (wID >> 4) + 1;

	if (pBuffer == NULL) {
		return 0;
	}

	hRsrc = FindResourceEx(hModule, RT_STRING, (LPCSTR)GroupId, wLangId);
	if (hRsrc) {
		HGLOBAL hStringSeg = LoadResource(hModule, hRsrc);
		PWSTR psz = (PWSTR)LockResource(hStringSeg);
		if (psz) {
			wID &= 0x0F;
			while (TRUE) {
				cch = *psz++;
				if (wID-- == 0) {
					break;
				}
				psz += cch;
			}

			if (cchBufferMax == 0) {
				*(PWSTR *)pBuffer = psz;
			} else {
				cchBufferMax--;
				if (cch > cchBufferMax) {
					cch = cchBufferMax;
				}

				RtlCopyMemory(pBuffer, psz, cch * sizeof(WCHAR));
			}
			UnlockResource(hStringSeg);
		}
	}

	if (cchBufferMax != 0) {
		pBuffer[cch] = 0;
	}
	return cch;
}

#define MAX_MSGTEXT     32
typedef int (WINAPI *PROC_SOFTMODALMESSAGEBOX)(PMSGBOXDATA lpmb);
typedef int (WINAPI *PROC_LOADSTRINGBASEEXW)(HINSTANCE hInstance, UINT uID, PWSTR lpBuffer, int nBufferMax, int LangId);

static int CustomBox(HWND hWnd, PCWSTR Text, PCWSTR Caption, UINT Type, ButtonW *pButtons, UINT cButtons, UINT dwTimeout, HINSTANCE hInstance, PCWSTR pszIcon, LANGID wLangId) {
	HMODULE hUser32 = LoadLibrary("user32");
	PROC_SOFTMODALMESSAGEBOX pSoftModalMessageBox = (PROC_SOFTMODALMESSAGEBOX)GetProcAddress(hUser32, "SoftModalMessageBox");
	PROC_LOADSTRINGBASEEXW LoadStringBaseExW = (PROC_LOADSTRINGBASEEXW)GetProcAddress(LoadLibrary("kernel32"), "LoadStringBaseExW");
	MSGBOXDATA mbd = {0};
	MSGBOXDATA *pmbd = &mbd;
	int ButtonIds[MAX_MSGBUTTONS] = {0};
	WCHAR *ButtonTexts[MAX_MSGBUTTONS] = {0};
	WCHAR TextBuffer[MAX_MSGBUTTONS][MAX_MSGTEXT] = {0};
	BOOL fCancel = FALSE;
	UINT i;
	ButtonW MsgBoxButtonNull = {0};

	if (pSoftModalMessageBox == 0) {
		return 0;
	}

	if (pButtons == NULL) {
		pButtons = &MsgBoxButtonNull;
		cButtons = 1;
	}

	if (cButtons == 0) {
		cButtons = 1;
	}

	// Max support 11 buttons
	if (cButtons > MAX_MSGBUTTONS) {
		cButtons = MAX_MSGBUTTONS;
	}

	for (i = 0; i < cButtons; i++) {
		ButtonIds[i] = pButtons[i].ID;
		ButtonTexts[i] = (WCHAR *)pButtons[i].label;
		if (ButtonIds[i] == IDCANCEL) {
			fCancel = TRUE;
		}

		// If user doesn't specify button text, try to load one from user32.dll resource
		if (ButtonTexts[i] == 0) {
			// Also can use the LoadStringBaseExW (available in Windows Vista or later) to load
			int n = (LoadStringBaseExW != NULL)
				? LoadStringBaseExW(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT, wLangId)
				: LoadStringEx(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT, wLangId);
			if (n == 0) {
				n = LoadStringW(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT);
			}

			ButtonTexts[i] = TextBuffer[i];
		}
	}

	mbd.cbSize = sizeof(MSGBOXPARAMSW);
	mbd.hwndOwner = hWnd;
	mbd.hInstance = hInstance;
	mbd.lpszText = Text;
	mbd.lpszCaption = Caption;
	mbd.dwStyle = Type;
	if (pszIcon != NULL) {
		mbd.lpszIcon = pszIcon;
		mbd.dwStyle &= (~MB_ICONMASK);
		mbd.dwStyle |= MB_USERICON;
	}

	if (LoadStringBaseExW == NULL) {
		pmbd = (MSGBOXDATA *)((UCHAR *)&mbd - sizeof(DWORD));
	}

	pmbd->wLanguageId = wLangId;
	pmbd->dwTimeout = (dwTimeout == 0) ? INFINITE : dwTimeout;
	pmbd->pidButton = ButtonIds;
	pmbd->ppszButtonText = ButtonTexts;
	pmbd->cButtons = cButtons;
	pmbd->DefButton = (mbd.dwStyle & MB_DEFMASK) >> 8;
	if (cButtons == 1 && pButtons[0].ID == IDOK) {
		pmbd->CancelId = IDOK;
	} else if (fCancel) {
		pmbd->CancelId = IDCANCEL;
		mbd.dwStyle |= MB_OKCANCEL;  // If MB_OK SoftModalMessageBox will return 1 always
	} else {
		mbd.dwStyle |= MB_OKCANCEL;  // If MB_OK SoftModalMessageBox will return 1 always
	}

	return pSoftModalMessageBox(&mbd);
}

/**
 * Parameters:
 *
 * - `hWnd` - Handle to the owner window of the message box to be created. If this parameter is NULL, the message box has no owner window.
 * - `Text` - Pointer to a null-terminated string that contains the message to be displayed.
 * - `Caption` - Pointer to a null-terminated string that contains the dialog box title. If this parameter is NULL, the default title Error is used.
 * - `Type` - Specifies the contents and behavior of the dialog box.
 * - `cButtons` - Specifies the count of pButtons
 * - `dwTimeout` - Specifies the time-out value to close the msgdlg automatically, in milliseconds. 0 means INFINITE
 * - `hInstance` - Handle to the module that contains the icon resource identified by the lpszIcon member, and the string resource identified by the lpszText or lpszCaption member.
 * - `lpszIcon` - Identifies an icon resource. This parameter can be either a null-terminated string or an integer resource identifier passed to the MAKEINTRESOURCE macro.
 * - `wLangId` - Specifies the language of default button text if not specify one in MSGBUTTON->ButtonText */
int message_box_ex(HWND hWnd, PCSTR Caption, PCSTR Text, UINT Type,
	Button *pButtons, UINT cButtons, UINT dwTimeout, HINSTANCE hInstance, PCSTR pszIcon, LANGID wLangId) {
	int ret;
	WCHAR *wText;
	WCHAR *wCaption;
	WCHAR TextBuffer[MAX_MSGBUTTONS][MAX_MSGTEXT] = {0};
	ButtonW wpButtons[MAX_MSGBUTTONS] = {0};
	UINT i;

	if (Text != NULL) {
		size_t TextLen = strlen(Text);
		wText = LocalAlloc(LPTR, (TextLen + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, Text, -1, wText, (int)TextLen);
	} else {
		wText = L"";
	}

	if (Caption != NULL) {
		size_t CaptionLen = strlen(Caption);
		wCaption = LocalAlloc(LPTR, (CaptionLen + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, Caption, -1, wCaption, (int)CaptionLen);
	} else {
		wCaption = L"";
	}

	for (i = 0; i < min(cButtons, MAX_MSGBUTTONS); i++) {
		wpButtons[i].ID = i + 1;
		wpButtons[i].result = pButtons[i].result;
		if (pButtons[i].label != NULL) {
			wpButtons[i].label = TextBuffer[i];
			MultiByteToWideChar(CP_ACP, 0, pButtons[i].label, -1, TextBuffer[i], MAX_MSGTEXT - 1);
		}
	}

	ret = CustomBox(hWnd, wText, wCaption, Type, wpButtons, cButtons, dwTimeout, hInstance, (PCWSTR)pszIcon, wLangId);
	if (Text != NULL)
		LocalFree(wText);

	if (Caption != NULL)
		LocalFree(wCaption);

	return ret;
}

int gui_message_box(ui_t *app, const char *title, const char *text, const Button *buttons, int numButtons) {
	return message_box_ex((app ? app->wnd : NULL), title, text, (numButtons == 1
		? MB_ICONINFORMATION
		: (numButtons > 2 ? MB_ICONEXCLAMATION : MB_ICONQUESTION)),
		(Button *)buttons, numButtons, 0, (app ? (HINSTANCE)app->app_data : NULL), (numButtons > 3 ? IDI_SHIELD : NULL), 0);
}

void gui_close(gui_info *ui) {
	if (ui) {
		if (ui->bar_info) {
			int i, count, x;
			menuitem_t *menuitem;
			for (i = 0; i < ui->bar_info->num_menus; i++) {
				count = ui->bar_info->menus[i].num_items;
				menuitem = ui->bar_info->menus[i].items;
				for (x = 0; x < count; x++) {
					DeleteObject(menuitem[i].hfont);
					if (menuitem[i].alphaKey != NULL)
						UnregisterHotKey(ui->wnd, menuitem[i].menu_id);
				}
				DestroyMenu(ui->bar_info->menus[i].hMenu);
			}
			free(ui->bar_info->menus);
			DestroyMenu(ui->bar_info->hMenubar);
			DeleteObject(ui->bar_info->font_info);
			free(ui->bar_info);
			ui->bar_info = NULL;
		}

		if (ui->buf) {
			free(ui->buf);
			ui->buf = NULL;
		}
		ui = NULL;
	}
}

int gui_loop(gui_info *ui) {
	while (PeekMessage(&ui->msg, NULL, 0, 0, PM_REMOVE)) {
		if (ui->msg.message == WM_QUIT || main_gui_shutdown)
			return -1;

		TranslateMessage(&ui->msg);
		DispatchMessage(&ui->msg);
	}

	InvalidateRect(ui->wnd, NULL, TRUE);
	return 0;
}

FORCEINLINE void gui_active(gui_info ui) {
	gui_handler(&ui);
}

void gui_destroy(gui_info ui) {
	if (ui.buf) {
		free(ui.buf);
		ui.buf = NULL;
	}

	if (ui.app && main_gui_shutdown)
		ExitProcess((int)ui.msg.wParam);
}

static void gui_querymenu(gui_info *ui) {
	menu_t menu;
	menuitem_t *menuitem;
	int i, id, x, count;
	long lfHeight = ui->bar_info->lfHeight;
	for (i = 0; i < ui->bar_info->num_menus; i++) {
		menu = ui->bar_info->menus[i];
		count = menu.num_items;
		menuitem = menu.items;
		for (x = 0; x < count; x++) {
			id = menuitem[x].menu_id;
			menuitem[x].hfont = CreateFont(ui->bar_info->lfHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				0, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (FF_MODERN | DEFAULT_PITCH), ui->bar_info->font_names);
			ModifyMenu(menu.hMenu, id, MF_BYCOMMAND | MF_OWNERDRAW, id, (LPTSTR)&menuitem[x]);
		}
	}

	DrawMenuBar(ui->wnd);
	ui->bar_info->bar_ready = true;
}

int gui_menufont(gui_info *ui, const char *font) {
	HFONT hf;
	HDC hdc;
	long lfHeight;

	hdc = GetDC(ui->wnd);
	lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 96);
	ReleaseDC(ui->wnd, hdc);
	if ((hf = CreateFont(lfHeight, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		0, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, (DEFAULT_PITCH | FF_MODERN), font))) {
		if (ui->bar_info->font_info)
			DeleteObject(ui->bar_info->font_info);

		ui->bar_info->lfHeight = lfHeight;
		ui->bar_info->font_names = (char *)font;
		ui->bar_info->font_info = hf;
		return 1;
	}

	return 0;
}

FORCEINLINE int gui_handler(gui_info *ui) {
	while (GetMessage(&ui->msg, NULL, 0, 0)) {
		if (ui->is_webview) {
			UINT message = ui->msg.message;
			if (message == WM_QUIT) {
				break;
			} else if (message == WM_COMMAND || message == WM_KEYDOWN || message == WM_KEYUP) {
				if (!webview_webview2_enabled) {
					HRESULT r = S_OK;
					IWebBrowser2 *webBrowser2;
					IOleObject *browser = *ui->web->priv.browser;
					if (browser->lpVtbl->QueryInterface(browser, iid_unref(&IID_IWebBrowser2),
						(void **)&webBrowser2) == S_OK) {
						IOleInPlaceActiveObject *pIOIPAO;
						if (browser->lpVtbl->QueryInterface(
							browser, iid_unref(&IID_IOleInPlaceActiveObject),
							(void **)&pIOIPAO) == S_OK) {
							r = pIOIPAO->lpVtbl->TranslateAccelerator(pIOIPAO, &ui->msg);
							pIOIPAO->lpVtbl->Release(pIOIPAO);
						}
						webBrowser2->lpVtbl->Release(webBrowser2);
					}

					if (r != S_FALSE)
						continue;
				}
			}
		}

		if (!IsDialogMessage(ui->wnd, &ui->msg)) {
			TranslateMessage(&ui->msg);
			DispatchMessage(&ui->msg);
		}
	}

	return (int)ui->msg.wParam;
}
#else
// window icon
#include X11_WINDOW_ICON
static char *Xt_menu_seperator = " ------ ";
static char *fallback[] = {
	"*variablewidth*font: -adobe-helvetica-medium-r-normal--*-120-*",
	"*monospaced*font: -*-courier-medium-r-*-*-14-*-*-*-*-*-*",
	"*sans-serif*font: -*-helvetica-medium-r-*-*-12-*-*-*-*-*-*",
	"*serif*font: -*-times-medium-r-*-*-12-*-*-*-*-*-*",
	"<Message>WM_PROTOCOLS: WMProtocols()\n",

/*General resources */
"webview*font: -*-helvetica-medium-r-normal-*-12-*-*-*-*-*-iso8859-*",

/* Class-specific resources */
"webview*MwCheck.background: white",
	"webview*MwMenuBar.box_width: 1",
	"webview*MwMenuButton.highlight_on_enter: False",
	"webview*MwTooltip.Label.background: LightYellow",
	"webview*MwTooltip.borderWidth:	1",
	"webview*MwRichtext.background: white",
	"webview*MwFrame.shadowWidth: 1",
	"webview*MwFrame.shadowType: Raised",
	"webview*MwFrame.allowResize: True",

/* Resources for individual widgets */
"webview*tooltip.translations:	#override	\\n\
	<Enter>: webview-highlight(1)	\\n\
	<Leave>: webview-unhighlight(0)",

"webview*html.MwTextField.translations:	#override	\\n\
	<Key>Return: form_done()		\\n\
	<Key>Escape: form_reset()		\\n\
	:<Key>Tab: form_next()		\\n\
	Ctrl<Key>n: form_next()		\\n\
	Ctrl<Key>p: form_previous()		\\n\
	<Btn1Down>: form_select()",

"webview*html.Text.translations: #override	\\n\
	:<Key>Tab: form_next()		\\n\
	<Btn1Down>: form_select()",

"webview*html.translations:	#override	\\n\
	<Key>space: 	page_down()	\\n\
	<Key>BackSpace: page_up()	\\n\
	<Key>Page_Down: page_down()	\\n\
	<Key>Page_Up: 	page_up()	\\n\
	<Key>Down: 		scroll_down()	\\n\
	<Key>Up: 		scroll_up()	\\n\
	:<Key>Left:		scroll_left()	\\n\
	:<Key>Right:	scroll_right()",
	NULL
};

#define GetNumFonts(X) 		X->num_fonts
#define LineHeight(X,Y,Z) 	((double)(X->size[1]+X->size[2])*(Y-Z))/(double)X->gwa.height
#define MenuName(X,Y) 		X->menus[Y].menu_name
#define MenuStart(X,Y) 		x_left+(X->menus[Y].x_start/(double)X->gwa.width)*(x_right-x_left)
#define MenuWidth(X,Y) 		((double)X->menus[Y].width/(double)window_info->gwa.width)*(x_right-x_left)
#define GetState(X) 		(X->state-1)

#define x_left 2
#define x_right 8.2
#define y_bot -1
#define y_top 68
#define left_click Button1
#define right_click Button3
#define open_menu 1
#define close_menu 3
#define run_command 4
#define left_menu_padding 10
#define top_menu_padding 0.1

struct Dimensions {
	//window
	unsigned int winMinWidth;
	unsigned int winMinHeight;
	//vertical space between lines
	unsigned int lineSpacing;
	unsigned int barHeight;
	//padding
	unsigned int pad_up;
	unsigned int pad_down;
	unsigned int pad_left;
	unsigned int pad_right;
	//button
	unsigned int btSpacing;
	unsigned int btMinWidth;
	unsigned int btMinHeight;
	unsigned int btLateralPad;
};

typedef struct ButtonData {
	const Button *button;
	GC *gc;
	XRectangle rect;
} ButtonData;

//these values can be changed to whatever you prefer
struct Dimensions dim = {400, 150, 5, 40, 25, 10, 30, 30, 20, 75, 25, 8};

typedef struct XPMstruct {
	char *pixdata;
	int width, height;
} XPMstruct;

//not the colour-depth, but the representation in memory
#define XPM_DEPTH 32

//fallback when no background-colour is given for transparency-supported XPM
#define TRANSPCOLOR 0x808080

//only suitable for XPMs with around max. 50 colours (single-character colour-lookups)
//max. 64-colour XPMs are supported! (one base64 char/colour)
static XPMstruct XPMtoPixels(char *source[], unsigned long bgcol) {
	int i, j, k, sourceindex, pixdataindex, width,
		height, colours, charpix, transpchar = 0xff;
	unsigned int colr[256], colg[256], colb[256];
	unsigned char colchar[256], colcode, colindex,
		transpr = (bgcol & 0xFF0000) / 65536,
		transpg = (bgcol & 0xFF00) / 256,
		transpb = bgcol & 0xFF;
	XPMstruct xpms;

	sscanf(source[0], "%d %d %d %d", &width, &height, &colours, &charpix);
	 //32 is sure to have enough room
	unsigned char *pixdata = malloc(width * height * XPM_DEPTH / 8);
	for (i = 0; i < colours; i++) {
		if (sscanf(source[1 + i], "%c%*s #%2X%2X%2X", &colchar[i], &colr[i], &colg[i], &colb[i]) != 4)
			transpchar = colchar[i];
	}

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			sourceindex = (i * width + j);
			colcode = source[colours + 1 + i][j];
			for (k = 0; k < colours; k++) {
				if (colcode == colchar[k])
					break;
			}

			colindex = k;
			pixdataindex = (i * width + j) * XPM_DEPTH / 8;
			//Blue 0x502040
			pixdata[pixdataindex + 0] = (colcode != transpchar) ? colb[colindex] : transpb;
			//Green
			pixdata[pixdataindex + 1] = (colcode != transpchar) ? colg[colindex] : transpg;
			//Red
			pixdata[pixdataindex + 2] = (colcode != transpchar) ? colr[colindex] : transpr;
			//Aplha - 0:fully transparent...0xFF:fully opaque /Xlib has no alpha-channel support, Xrender has transparency-support
			pixdata[pixdataindex + 3] = (colcode == transpchar) ? 0x00 : 0xff;
		}
	}

	xpms.pixdata = pixdata;
	xpms.width = width;
	xpms.height = height;
	return xpms;
}

static void gui_seticon(gui_info *ui, char *xpm[]) {
	ui->icon_set = 1;
	XPMstruct xpms = XPMtoPixels(xpm, TRANSPCOLOR);
	XImage *iconimg = XCreateImage(ui->dpy, CopyFromParent, DefaultDepth(ui->dpy, ui->screen),
		ZPixmap, 0, xpms.pixdata, xpms.width, xpms.height, XPM_DEPTH, 0);

	XInitImage(iconimg);
	Pixmap icon = XCreatePixmap(ui->dpy, ui->win, xpms.width, xpms.height, DefaultDepth(ui->dpy, ui->screen));
	if (!ui->gc)
		ui->gc = XCreateGC(ui->dpy, ui->win, 0, 0);

	XPutImage(ui->dpy, icon, ui->gc, iconimg, 0, 0, 0, 0, xpms.width, xpms.height);
	XWMHints *hints = XAllocWMHints();
	hints->flags = IconPixmapHint;
	hints->icon_pixmap = icon;
	XSetWMHints(ui->dpy, ui->win, hints);
	XDestroyImage(iconimg);
	XUnmapWindow(ui->dpy, ui->win);
	XMapWindow(ui->dpy, ui->win);
	XFree(hints);
}

static int load_font(XFontStruct *font_info, char *font, Display *dpy, GLuint font_base, int *size) {
	font_info = XLoadQueryFont(dpy, font);
	size[0] = font_info->ascent;
	size[1] = font_info->descent;
	if (!font_info) {
		return 1;
	}

	glXUseXFont(font_info->fid, font_info->min_char_or_byte2, font_info->max_char_or_byte2 - font_info->min_char_or_byte2 + 1, font_base + font_info->min_char_or_byte2);
	return 0;
}

static int print_string(GLuint font_base, char *s) {
	if (!glIsList(font_base)) {
		return 1;
	}
	glPushAttrib(GL_TEXTURE_BIT);
	glListBase(font_base);
	glCallLists(strlen(s), GL_UNSIGNED_BYTE, (GLubyte *)s);
	glPopAttrib();
	return 0;
}

static int drawMenubar(menu_bar_t *window_info) {
	int i = 0, error = 0;
	double x, y;
	y = y_top - ((double)window_info->size[1] / (double)window_info->gwa.height) * (y_top - y_bot);
	char name[max_font_name_length * 2];
	while (i < (window_info->num_menus)) {
		glColor3f(0, 0, 0);
		x = MenuStart(window_info, i);
		glRasterPos2f(x, y);
		snprintf(name, sizeof(name) - 1, " %s ", MenuName(window_info, i));
		error += print_string(window_info->font_lists, name);
		++i;
	}

	y = y_top - LineHeight(window_info, y_top, y_bot);
	glColor3f(1, 1, 1);
	glBegin(GL_POLYGON);
	glVertex3f(x_left, y_top, 0);  glVertex3f(x_right, y_top, 0);
	glVertex3f(x_right, y, 0);   glVertex3f(x_left, y, 0);
	glEnd();
	return error;
}

static int drawMenu(menu_bar_t *window_info) {
	int error = 0, index = 0;
	double y, line_height, x;
	line_height = LineHeight(window_info, y_top, y_bot);
	y = (double)(2 * window_info->size[1] + window_info->size[2]);
	y = y * (y_top - y_bot);
	y = y / (double)window_info->gwa.height;
	y = y_top - y;
	x = MenuStart(window_info, GetState(window_info));
	char name[max_font_name_length * 2];
	while (index < (window_info->menus[GetState(window_info)].num_items)) {
		if (index == window_info->menus[GetState(window_info)].active) {
			glColor3f(1, 1, 1);
		} else {
			glColor3f(0, 0, 0);
		}
		glRasterPos2f(x, y);
		snprintf(name, sizeof(name) - 1, " %s ", window_info->menus[GetState(window_info)].items[index].item_name);
		error += print_string(window_info->font_lists, name);
		y -= (line_height);
		++index;
	}

	glColor3f(0.7, 0.7, 0.7);
	y = y_top - (window_info->menus[GetState(window_info)].num_items + 1) * line_height;
	glBegin(GL_POLYGON);
	glVertex3f(x, y, 0);
	glVertex3f(x + MenuWidth(window_info, GetState(window_info)) + 0.1, y, 0);
	glVertex3f(x + MenuWidth(window_info, GetState(window_info)) + 0.1, y_top - line_height, 0);
	glVertex3f(x, y_top - line_height, 0);
	glEnd();
	return error;
}

static int draw(menu_bar_t *window_info) {
	int error = 0;
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(x_left, x_right, y_bot, y_top, 1., 20.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);

	error = drawMenubar(window_info);

	if (window_info->state != 0) {
		error += drawMenu(window_info);
	}

	return error;
}

static int whichMenu(menu_bar_t *window_info, double x) {
	int i = window_info->num_menus - 1;
	while (index > 0 && (MenuStart(window_info, i) > x)) {
		--i;
	}
	return i + 1;
}

static int whichActive(menu_bar_t *window_info, double x, double y) {
	int index = 0;
	double y_cat, line_height, x_min, x_max;
	line_height = LineHeight(window_info, y_top, y_bot);
	y_cat = y_top - line_height;
	x_min = MenuStart(window_info, GetState(window_info));
	x_max = x_min + MenuWidth(window_info, GetState(window_info));
	if (y > y_cat || x < x_min || x > x_max) {
		return -1;
	}
	y_cat -= line_height;
	while (x > x_min && x < x_max && y < y_cat && index < window_info->menus[GetState(window_info)].num_items) {
		y_cat -= (line_height);
		++index;
	}
	return (index != window_info->menus[GetState(window_info)].num_items) ? index : -2;
}

static void WMProtocols(Widget w, XEvent *ev, String *params, Cardinal *nparams) {
	if (ev->type == ClientMessage
		&& !strcmp(XGetAtomName(XtDisplay(w), ev->xclient.message_type), "WM_PROTOCOLS")) {
	}
}

void verify_form(__GUI_FIELD__) {
	gui_info *ui = (gui_info *)client;
	ui_field form, *fill = (ui_field *)ui->app->app_array;
	int len, i, numFields = ui->app->code;
	ui_form_cb verify = (ui_form_cb)ui->user_data;
	char *value;
	Widget field;

	for (i = 0; i < numFields; i++) {
		form = fill[i];
		field = (Widget)form.index;
		char error[100] = {0};
		if (str_field_valid(field, form)) {
		} else {
			switch (form.kind) {
				case field_number:
					snprintf(error, sizeof(error), "Error: number must be between %d or %d digits",
						form.min, form.max);
					break;
				case field_text:
					snprintf(error, sizeof(error), "Error: length overflow %d or underflow %d",
						form.max, form.min);
					break;
				case field_secret:
					snprintf(error, sizeof(error), "Error: secret aleast 1 cap, 1 num and minimum %d",
						form.min);
					break;
				case field_email:
					snprintf(error, sizeof(error), "Error: invalid Email");
					break;
			}

			XtVaSetValues(ui->statusLine, XtNlabel, error, XtNforeground, 0xFF0000, NULL);
			XtVaSetValues(field, XtNborderColor, 0xFF0000, NULL);
			return;
		}

		XtVaSetValues(ui->statusLine, XtNlabel, "", NULL);
		XtVaSetValues(field, XtNborderColor, 0x008000, NULL);
		if (verify) {
			char status[260] = {0};
			if (verify(ui->app, form.ID, value, status)) {
				XtVaSetValues(ui->statusLine, XtNlabel, status, NULL);
			} else {
				// Error
				XtVaSetValues(ui->statusLine, XtNlabel, status, NULL);
				return;
			}
		}
	}

	gui_cancel(self);
}

FORCEINLINE void cancel_form(__GUI_FIELD__) {
	gui_cancel(self);
}

static FORCEINLINE void gain_ebitable_field(__GUI_FIELD__) {
	XtVaSetValues(self, XtNdisplayCaret, True, NULL);
}

static FORCEINLINE void lose_ebitable_field(__GUI_FIELD__) {
	XtVaSetValues(self, XtNdisplayCaret, False, NULL);
}

static void tab_ebitable_field(__GUI_FIELD__) {
	gui_info *ui = (gui_info *)client;
	ui_field *fill = (ui_field *)ui->app->app_array;
	ui_form_t field;
	int i, numFields = ui->app->code;
	for (i = 0; i < numFields; i++) {
		if (self == fill[i].index)
			break;
	}

	XtVaSetValues(self, XtNdisplayCaret, False, NULL);
	field = ((i + 1) == numFields) ? fill[0].index : fill[i + 1].index;
	XtSetKeyboardFocus((Widget)ui->app->app_data, field);
	XtVaSetValues(field, XtNdisplayCaret, True, NULL);
}

void verify_field(__GUI_FIELD__) {
	Arg wargs[10];
	gui_info *ui = (gui_info *)client;
	TextFieldReturnStruct *ret = (TextFieldReturnStruct *)data;
	printf("changed: string = %s\n", ret->string);
	//	XtSetArg(wargs[0], XtNlabel, "Error: invalid input!");
	//	XtSetValues(ui->statusLine, wargs, 1);
}

Widget Xt_field(Widget window, Widget alignto, char *area, float x, float y, float width,
	ui_field_type kind, gui_info *ui) {
	Widget text = XtVaCreateManagedWidget("sans-serif", textfieldWidgetClass, window,
		XtNwidth, width,
		XtNstring, area,
		XtNborder, 0,
		XtNinsertPosition, 0,
		XtNdisplayCaret, False,
		XtNallowSelection, True,
		XtNresizable, True,
		XtNeditable, True,
		XtNecho, (kind == field_secret ? False : True),
		XtNborderColor, 0xDAA520,
		XtNx, x,
		XtNy, y,
		XtNheight, 30,
		XtNleft, XawChainLeft,
		XtNright, XawRubber,
		XtNtop, XawChainTop,
		XtNbottom, XawRubber,
		XtNfromVert, alignto,
		NULL);

	XtAddCallback(text, XtNfocusCallback, gain_ebitable_field, NULL);
	XtAddCallback(text, XtNgainPrimaryCallback, gain_ebitable_field, NULL);
	XtAddCallback(text, XtNlosingFocusCallback, lose_ebitable_field, NULL);
	XtAddCallback(text, XtNlosePrimaryCallback, lose_ebitable_field, NULL);
	return text;
}

Widget Xt_text_field(gui_info *gui, ui_field_type kind, char *label, char *field,
	float x, float y, float width, uintptr_t tag) {
	Widget text;
	return text;
}

Widget Xt_form_button(gui_info *gui, char *title, XtCallbackProc action, int x) {
	Widget button;
	if (x == 1)
		button = XtVaCreateManagedWidget("command",
			commandWidgetClass, (Widget)gui->app->app_data,
			XtNlabel, title,
			XtNwidth, 80,
			XtNheight, 18,
			XtNborder, 1,
			XtNfont, gui->font,
			XtNborderColor, 0xDAA520,
			XtNborderWidth, 2,
			XtNleft, XawChainRight,
			XtNright, XawChainRight,
			XtNtop, XawChainRight,
			XtNbottom, XawChainRight,
			XtNfromVert, (Widget)gui->app->app_array,
			NULL);
	else
		button = XtVaCreateManagedWidget("command",
			commandWidgetClass, (Widget)gui->app->app_data,
			XtNlabel, title,
			XtNwidth, 80,
			XtNheight, 18,
			XtNborder, 1,
			XtNfont, gui->font,
			XtNborderColor, 0xDAA520,
			XtNborderWidth, 2,
			XtNleft, XawChainRight,
			XtNright, XawChainRight,
			XtNtop, XawChainRight,
			XtNbottom, XawChainRight,
			XtNfromHoriz, (Widget)gui->app->app_array,
			NULL);

	XtAddCallback(button, XtNcallback, action, gui);
	gui->app->app_array = (void **)button;
	return button;
}

int gui_form(gui_info *ui, const char *title, Form *fill, int numFields, ui_form_cb verify) {
	int i, y = 0, max_width = 0, spacing = 30;
	int argc = 0;
	char **argv = NULL;

	/* calculate form width based off longest field width */
	for (i = 0; i < numFields; i++) {
		if (fill[i].width > max_width)
			max_width = fill[i].width;
	}

	ui->skip_resize = true;
	ui->title = title;
	ui->width = max_width + 20;
	/* calculate form height based off number of text fields provided */
	ui->height = numFields * 48;
	ui->user_data = verify;
	ui->app->code = numFields;
	ui->topLevel = XtAppInitialize(&ui->app_con, title, NULL, 0,
		&argc, argv, fallback, NULL, 0);
	XtResizeWidget(ui->topLevel, ui->width, ui->height, 0);

	Widget text = NULL, form = XtCreateManagedWidget("form", formWidgetClass, ui->topLevel, NULL, 0);
	ui->font = XLoadQueryFont(XtDisplayOfObject(form), "lucidasans-8");
	ui->font_button = XLoadQueryFont(XtDisplayOfObject(form), "lucidasans-bold-8");
	for (i = 0; i < numFields; i++) {
		/* Setup spacing between each `textfield` with `caption/label` */
		y += spacing;
		if (fill[i].caption) {
			text = XtVaCreateManagedWidget("label", labelWidgetClass, form,
				XtNlabel, fill[i].caption,
				XtNborder, 0,
				XtNfont, ui->font,
				XtNborderWidth, 0,
				XtNheight, 9,
				XtNx, 6,
				XtNy, y - 12,
				XtNleft, XtChainLeft,
				XtNfromVert, text, NULL);
		}

		text = XtVaCreateManagedWidget("sans-serif", textfieldWidgetClass, form,
			XtNwidth, fill[i].width,
			XtNstring, fill[i].value,
			XtNborder, 0,
			XtNinsertPosition, 0,
			XtNdisplayCaret, False,
			XtNallowSelection, True,
			XtNeditable, True,
			XtNecho, (fill[i].kind != field_secret ? True : False),
			XtNborderColor, 0xDAA520,
			XtNx, 5,
			XtNy, y - 5,
			XtNheight, 18,
			XtNleft, XtChainLeft,
			XtNfromVert, text,
			NULL);

		XtAddCallback(text, XtNactivateCallback, verify_field, ui);
		XtAddCallback(text, XtNvalueChangedCallback, verify_field, ui);
		XtAddCallback(text, XtNfocusCallback, gain_ebitable_field, ui);
		XtAddCallback(text, XtNgainPrimaryCallback, gain_ebitable_field, ui);
		XtAddCallback(text, XtNlosingFocusCallback, lose_ebitable_field, ui);
		XtAddCallback(text, XtNlosePrimaryCallback, lose_ebitable_field, ui);
		XtAddCallback(text, XtNtabCallback, tab_ebitable_field, ui);
		//XtAddCallback(text, XtNmodifyVerifyCallback, ebitable_field, ui);
		fill[i].index = text;
	}

	ui->app->app_data = form;
	ui->app->app_array = (void **)text;

	Xt_form_button(ui, "Confirm", verify_form, 1);
	text = Xt_form_button(ui, "Cancel", cancel_form, 1);

	/* Setup statusline area in form for `error` feedback*/
	ui->statusLine = XtVaCreateManagedWidget("label", labelWidgetClass, form,
		XtNwidth, ui->width - 20,
		XtNlabel, "Fill out",
		XtNforeground, 0x008000,
		XtNfont, ui->font,
		XtNborder, 0,
		XtNborderWidth, 0,
		XtNheight, 10,
		XtNfromVert, text, NULL);

	/* Store provided `Form` for `verify_form` button click verification process */
	ui->app->name = ui->title;
	ui->app->wnd = ui->topLevel;
	ui->app->gui = ui;
	ui->wnd = ui->topLevel;
	ui->app->app_array = (void **)fill;
	return 1;
}

static Boolean source_changed = FALSE;
void SourceChanged(Widget w, XtPointer  junk, XtPointer garbage) {
	XtRemoveCallback(w, XtNcallback, SourceChanged, NULL);
	source_changed = TRUE;
}

void ResetSourceChanged(Widget widget) {
	XtAddCallback(XawTextGetSource(widget), XtNcallback, SourceChanged, NULL);
	source_changed = FALSE;
}

void gui_file(__GUI_FILE__) {
	gui_info ui = {0};
	Cardinal num_args = 0;
	Arg arglist[10];
	int argc = 0;
	char **argv = NULL;

	if (data == NULL) {
		gui_cancel(self);
		return;
	}

	ui.topLevel = XtAppInitialize(&ui.app_con, data, NULL, 0,
		&argc, argv, fallback, NULL, 0);

	XtSetArg(arglist[num_args], XtNtype, XawAsciiFile); num_args++;
	XtSetArg(arglist[num_args], XtNeditType, XawtextEdit); num_args++;
	XtSetArg(arglist[num_args], XtNwidth, 500); num_args++;
	XtSetArg(arglist[num_args], XtNheight, 500); num_args++;
	XtSetArg(arglist[num_args], XtNshowGrip, True); num_args++;
	XtSetArg(arglist[num_args], XtNstring, data); num_args++;
	Widget textwindow = XtCreateManagedWidget("editWindow", asciiTextWidgetClass,
		ui.topLevel, arglist, num_args);

	XtSetValues(textwindow, arglist, num_args);
	ResetSourceChanged(textwindow);

	ui.wnd = ui.topLevel;
	ui.app->wnd = ui.topLevel;
	ui.app->app_data = textwindow;
	ui.app->name = data;
	ui.app->gui = &ui;

	gui_info *gui = (gui_info *)client;
	XtUnmapWidget(gui->topLevel);
	gui_cancel(self);
	gui_active(ui);
	gui_destroy(ui);
}

void gui_save_dialog(__GUI_MENU__) {
	//XawAsciiSaveAsFile(XawTextGetSource(self), data);
	//ResetSourceChanged(self);
}

void gui_open_dialog(__GUI_MENU__) {
	gui_info ui = {0};
	char *filter = "*";
	char *dir = "./";
	char *initial = "";

	int argc = 0;
	char **argv = NULL;

	ui.topLevel = XtAppInitialize(&ui.app_con, "FilePrompt", NULL, 0,
		&argc, argv, fallback, NULL, 0);
	ui.width = 400;
	ui.height = 400;
	XtResizeWidget(ui.topLevel, ui.width, ui.height, 0);

	Widget fileSelect = XtVaCreateManagedWidget("fileSelector",
		fileSelectWidgetClass, ui.topLevel, NULL, 0);
	if (data)
		XtAddCallback(fileSelect, XtNcallback, (XtCallbackProc)data, &ui);
	else
		XtAddCallback(fileSelect, XtNcallback, (XtCallbackProc)gui_file, &ui);

	FileSelectSet(fileSelect, dir, filter, initial);
	ui.app->app_data = (void *)fileSelect;
	ui.app->name = "Select File";
	ui.app->wnd = ui.topLevel;
	ui.app->gui = &ui;
	gui_active(ui);
	gui_destroy(ui);
}

void gui_cancel(ui_wnd_t self) {
	Display *display = XtDisplayOfObject(self);
	Window win = XtWindowOfObject(self);
	XEvent ev;

	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = win;
	ev.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", true);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", false);
	ev.xclient.data.l[1] = CurrentTime;
	XSendEvent(display, win, False, NoEventMask, &ev);
	XSync(display, False);
}

void gui_active(gui_info ui) {
	Display *ldpy = XtDisplayOfObject((Widget)ui.app->app_data);
	XtAppContext context = XtWidgetToApplicationContext((Widget)ui.app->app_data);
	ui.dpy = ldpy;
	XtRealizeWidget(ui.app->wnd);

	XtActionsRec fileprompt_actions[] = {
		{"WMProtocols", WMProtocols},
	};

	XtAppAddActions(context,
		fileprompt_actions, XtNumber(fileprompt_actions));

	/* set up to handle quits */
	XInternAtom(ldpy, "WM_PROTOCOLS", False);
	Atom code = XInternAtom(ldpy, "WM_DELETE_WINDOW", False);
	XtOverrideTranslations(ui.app->wnd,
		XtParseTranslationTable("<Message>WM_PROTOCOLS: WMProtocols()"));

	XSizeHints hints;
	ui.win = XtWindow(ui.app->wnd);
	(void)XSetWMProtocols(ldpy, ui.win, (Atom *)&code, 1);

	if (ui.skip_resize) {
		hints.flags = PSize | PMinSize | PMaxSize;
		hints.min_width = hints.max_width = hints.base_width = ui.width;
		hints.min_height = hints.max_height = hints.base_height = ui.height;
		XSetWMNormalHints(ldpy, ui.win, &hints);
	}

	if (!ui.icon_set) {
		ui.icon_set = 1;
		MwSetIcon(ui.topLevel, icon_32x32);
	}

	XStoreName(ldpy, ui.win, ui.app->name);
	XMapWindow(ui.dpy, ui.win);
	XEvent ev;
	for (;;) {
		XtAppNextEvent(context, &ev);
		XtDispatchEvent(&ev);
		if (ev.xclient.type == ClientMessage && ev.xclient.data.l[0] == code)
			break;
	}
	XtUnrealizeWidget(ui.app->wnd);
}

int gui_menu(gui_info *ui, int num_menu, menuitem_t *items, int number_items, int menu_id, char *name) {
	int r;
	menu_t *menu = &(ui->bar_info->menus[num_menu]);
	menu->num_items = number_items;
	menu->selected = none_selected;
	menu->menu_id = menu_id;
	menu->items = items;
	//menu->items = (menuitem_t *)calloc(1, menu->num_items * sizeof(menuitem_t));
	//if (memcpy(menu->items, items, menu->num_items * sizeof(menuitem_t)) == NULL) {
	//	return 0;
	//}

	if (!(r = snprintf(menu->menu_name, sizeof(menu->menu_name) - 1, "%s", name)))
		XtAppError(ui->app_con, "\tMenu failed\n");

	return r;
}

static void gui_querymenu(gui_info *ui) {
	int error = 0, index = 0, item_index = 0;
	double x = 0, width = 0, temp;
	XFontStruct *menu_font = XLoadQueryFont(ui->dpy, ui->bar_info->font_names);

	while (index < (ui->bar_info->num_menus)) {
		ui->bar_info->menus[index].x_start = x;
		x += XTextWidth(menu_font, ui->bar_info->menus[index].menu_name, strlen(ui->bar_info->menus[index].menu_name));
		ui->bar_info->menus[index].x_end = x;
		x += left_menu_padding;
		while (item_index < ui->bar_info->menus[index].num_items) {
			if (!ui->bar_info->menus[index].items[item_index].item_name
				&& !ui->bar_info->menus[index].items[item_index].action) {
				ui->bar_info->menus[index].items[item_index].item_name = Xt_menu_seperator;
				temp = XTextWidth(menu_font, Xt_menu_seperator, strlen(Xt_menu_seperator));
			} else {
				temp = XTextWidth(menu_font, ui->bar_info->menus[index].items[item_index].item_name,
					strlen(ui->bar_info->menus[index].items[item_index].item_name));
				temp += 4;
			}
			width = (temp > width) ? temp : width;
			++item_index;
		}
		item_index = 0;
		ui->bar_info->menus[index].width = width;
		++index;
	}
	ui->bar_info->bar_ready = true;
}

int gui_menufont(gui_info *ui, const char *font) {
	ui->bar_info->font_names = strdup(font);
	ui->bar_info->font_lists = glGenLists(256);
	if (!glIsList(ui->bar_info->font_lists)) {
		XtAppError(ui->app_con, "\tfont list failure\n");
		return 0;
	}

	if (load_font(ui->bar_info->font_info, ui->bar_info->font_names,
		ui->dpy, ui->bar_info->font_lists, &ui->bar_info->size[1]) != 0) {
		XtAppError(ui->app_con, "\tfont load failure\n");
		return 0;
	}

	return 1;
}

static void gui_free(gui_info *ui) {
	int i;
	if (ui) {
		if (ui->bar_info) {
			free(ui->bar_info->font_names);
			free(ui->bar_info->menus);
			free(ui->bar_info->font_info);
			free(ui->bar_info->size);
			free(ui->bar_info);
			ui->bar_info = NULL;
		}

		if (ui->glc) {
			glXMakeCurrent(ui->dpy, None, NULL);
			glXDestroyContext(ui->dpy, ui->glc);
		}

		XDestroyWindow(ui->dpy, ui->win);
		if (ui->buf) {
			free(ui->buf);
			ui->buf = NULL;
		}

		ui = NULL;
	}
}

int gui_menubar(gui_info *ui, int numof_menus) {
	int index = 0;
	if ((ui->bar_info = (menu_bar_t *)calloc(1, sizeof(menu_bar_t)))) {
		ui->bar_info->num_menus = numof_menus;
		ui->bar_info->menus = (menu_t *)calloc(1, GetNumMenus(ui->bar_info) * sizeof(menu_t));
		ui->bar_info->num_fonts = 1;
		ui->bar_info->font_info = (XFontStruct *)malloc(GetNumFonts(ui->bar_info) * sizeof(XFontStruct));
		ui->bar_info->size = (int *)malloc(GetNumFonts(ui->bar_info) * 2 * sizeof(int));
		ui->bar_info->state = 0;
		return 1;
	}

	return 0;
}

int gui_handler(gui_info *ui) {
	int buttoncase, click_x, click_y, error, check;
	double x_limit, y_limit, x, y;
	ui_t *app = ui->app;
	if (!ui->bar_info->bar_ready)
		gui_querymenu(ui);

	glEnable(GL_DEPTH_TEST);
	while (1) {
		XNextEvent(ui->dpy, &ui->xev);
		switch (ui->xev.type) {
			case MotionNotify:
				if (ui->bar_info->state != 0) {
					x = x_left + ((double)ui->xev.xmotion.x / (double)ui->bar_info->gwa.width) * (x_right - x_left);
					y = y_top - ((double)ui->xev.xmotion.y / (double)ui->bar_info->gwa.height) * (y_top - y_bot);
					check = ui->bar_info->menus[GetState(ui->bar_info)].active;
					ui->bar_info->menus[GetState(ui->bar_info)].active = whichActive(ui->bar_info, x, y);
					if (check != ui->bar_info->menus[GetState(ui->bar_info)].active) {
						error = draw(ui->bar_info);
						glXSwapBuffers(ui->dpy, ui->win);
					}
				}
				break;
			case ButtonPress:
				x_limit = x_left + (ui->bar_info->menus[ui->bar_info->num_menus - 1].x_end / (double)ui->bar_info->gwa.width) * (x_right - x_left);
				y_limit = y_top - LineHeight(ui->bar_info, y_top, y_bot);
				x = x_left + ((double)ui->xev.xbutton.x / (double)ui->bar_info->gwa.width) * (x_right - x_left);
				click_x = (x < 0) ? -1 : (int)x;
				y = y_bot + ((double)(ui->bar_info->gwa.height) - (double)ui->xev.xbutton.y) * ((double)y_top - (double)y_bot) / ((double)ui->bar_info->gwa.height);
				click_y = (y < 0) ? -1 : (int)y;
				buttoncase = open_menu * (ui->xev.xbutton.button == left_click) * (ui->bar_info->state == 0) * (x < x_limit) * (y > y_limit);
				buttoncase += close_menu * (ui->bar_info->state != 0) * (whichActive(ui->bar_info, x, y) < 0);
				buttoncase += run_command * (ui->bar_info->state != 0) * (whichActive(ui->bar_info, x, y) >= 0);
				switch (buttoncase) {
					case open_menu:
						ui->bar_info->state = whichMenu(ui->bar_info, x);
						break;
					case close_menu:
						ui->bar_info->state = 0;
						break;
					case run_command:
						memset(app, 0, sizeof(ui_t));
						app->wnd = ui->topLevel;
						app->gui = ui;
						menuitem_t menu_active = ui->bar_info->menus[GetState(ui->bar_info)]
							.items[ui->bar_info->menus[GetState(ui->bar_info)].active];
						if (menu_active.action)
							menu_active.action(app, menu_active.data);
						break;
					default:
						break;
				}
			case Expose:
				XGetWindowAttributes(ui->dpy, ui->win, &(ui->bar_info->gwa));
				glViewport(0, 0, ui->bar_info->gwa.width, ui->bar_info->gwa.height);
				error = draw(ui->bar_info);
				if (error != 0) {
					fprintf(stderr, "\tfont failure: %d\n", error);
					return 0;
				}
				glXSwapBuffers(ui->dpy, ui->win);
				break;
			case ClientMessage:
				if (ui->xev.xclient.data.l[0] == ui->wmDeleteMessage) {
					return 0;
				}
				break;
			default:
				break;
		}
	}
	return 0;
}

static void setWindowTitle(const char *title, const Window *win, Display *dpy) {
	Atom wm_Name = XInternAtom(dpy, "_NET_WM_NAME", False);
	Atom utf8Str = XInternAtom(dpy, "UTF8_STRING", False);

	Atom winType = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	Atom typeDialog = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);

	XChangeProperty(dpy, *win, wm_Name, utf8Str, 8, PropModeReplace, (const unsigned char *)title, (int)strlen(title));
	XChangeProperty(dpy, *win, winType, XA_ATOM,
		32, PropModeReplace, (unsigned char *)&typeDialog,
		1);
}

static void split(const char *text, const char *seps, char ***str, int *count) {
	char *last, *tok, *data;
	int i;
	*count = 0;
	data = strdup(text);

	for (tok = strtok(data, seps); tok != NULL; tok = strtok(NULL, seps))
		(*count)++;

	free(data);
	fflush(stdout);
	data = strdup(text);
	*str = (char **)malloc((size_t)(*count) * sizeof(char *));

	for (i = 0, tok = strtok(data, seps); tok != NULL; tok = strtok(NULL, seps), i++)
		(*str)[i] = strdup(tok);
	free(data);
}

static void computeTextSize(XFontSet *fs, char **texts, int size, unsigned int spaceBetweenLines,
	unsigned int *w, unsigned  int *h) {
	int i;
	XRectangle rect = {0,0,0,0};
	*h = 0;
	*w = 0;
	for (i = 0; i < size; i++) {
		Xutf8TextExtents(*fs, texts[i], (int)strlen(texts[i]), &rect, NULL);
		*w = (rect.width > *w) ? (rect.width) : *w;
		*h += rect.height + spaceBetweenLines;
		fflush(stdin);
	}
}

static void createGC(GC *gc, const Colormap *cmap, Display *dpy, const  Window *win,
	unsigned char red, unsigned char green, unsigned char blue) {
	float coloratio = (float)65535 / 255;
	XColor color;
	*gc = XCreateGC(dpy, *win, 0, 0);
	memset(&color, 0, sizeof(color));
	color.red = (unsigned short)(coloratio * red);
	color.green = (unsigned short)(coloratio * green);
	color.blue = (unsigned short)(coloratio * blue);
	color.flags = DoRed | DoGreen | DoBlue;
	XAllocColor(dpy, *cmap, &color);
	XSetForeground(dpy, *gc, color.pixel);
}

static bool isInside(int x, int y, XRectangle rect) {
	if (x < rect.x || x >(rect.x + rect.width) || y < rect.y || y >(rect.y + rect.height))
		return false;
	return true;
}

int gui_message_box(ui_t *app, const char *title, const char *text, const Button *buttons, int numButtons) {
	gui_info ui = {0};
	// convert the text in list (to draw in multiply lines)
	char **text_splitted = NULL;
	int textLines = 0;
	split(text, "\n", &text_splitted, &textLines);

	ui.dpy = XOpenDisplay(NULL);
	if (ui.dpy == NULL) {
		fprintf(stderr, "Error opening display display.");
		return -1;
	}

	ui.screen = DefaultScreen(ui.dpy);
	ui.win = XCreateSimpleWindow(ui.dpy, RootWindow(ui.dpy, ui.screen), 0, 10, 400, 120, 0,
		BlackPixel(ui.dpy, ui.screen), WhitePixel(ui.dpy, ui.screen));

	XSelectInput(ui.dpy, ui.win, ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
	XMapWindow(ui.dpy, ui.win);

	//allow windows to be closed by pressing cross button (but it wont close - see ClientMessage on switch)
	Atom WM_DELETE_WINDOW = XInternAtom(ui.dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(ui.dpy, ui.win, &WM_DELETE_WINDOW, 1);

	// create the gc for drawing text
	XGCValues gcValues;
	gcValues.font = XLoadFont(ui.dpy, "7x13");
	gcValues.foreground = BlackPixel(ui.dpy, ui.screen);
	ui.gc = XCreateGC(ui.dpy, ui.win, GCFont + GCForeground, &gcValues);
	XUnmapWindow(ui.dpy, ui.win);

	// create fontset
	char **missingCharset_list = NULL;
	int i, missingCharset_count = 0;
	XFontSet fs;
	fs = XCreateFontSet(ui.dpy,
		"-*-*-medium-r-*-*-*-140-75-75-*-*-*-*",
		&missingCharset_list, &missingCharset_count, NULL);

	if (missingCharset_count) {
		fprintf(stderr, "Missing charsets :\n");
		for (i = 0; i < missingCharset_count; i++) {
			fprintf(stderr, "%s\n", missingCharset_list[i]);
		}
		XFreeStringList(missingCharset_list);
		missingCharset_list = NULL;
	}

	Colormap cmap = DefaultColormap(ui.dpy, ui.screen);

	//resize the window according to the text size
	unsigned int winW, winH;
	unsigned int textW, textH;

	//calculate the ideal window's size
	computeTextSize(&fs, text_splitted, textLines, dim.lineSpacing, &textW, &textH);
	unsigned int newWidth = textW + dim.pad_left + dim.pad_right;
	unsigned int newHeight = textH + dim.pad_up + dim.pad_down + dim.barHeight;
	winW = (newWidth > dim.winMinWidth) ? newWidth : dim.winMinWidth;
	winH = (newHeight > dim.winMinHeight) ? newHeight : dim.winMinHeight;

	//set windows hints
	XSizeHints hints;
	hints.flags = PSize | PMinSize | PMaxSize;
	hints.min_width = hints.max_width = hints.base_width = winW;
	hints.min_height = hints.max_height = hints.base_height = winH;

	XSetWMNormalHints(ui.dpy, ui.win, &hints);
	gui_seticon(&ui, icon_32x32);

	GC barGC;
	GC buttonGC;
	GC buttonGC_underPointer;
	GC buttonGC_onClick;                               // GC colors
	createGC(&barGC, &cmap, ui.dpy, &ui.win, RGB_WHITE);
	createGC(&buttonGC, &cmap, ui.dpy, &ui.win, RGB_GOLDEN_ROD);
	createGC(&buttonGC_underPointer, &cmap, ui.dpy, &ui.win, RGB_SILVER);
	createGC(&buttonGC_onClick, &cmap, ui.dpy, &ui.win, RGB_DIM_GRAY);

	/* setup the buttons data */
	ButtonData *btsData = (ButtonData *)malloc((size_t)numButtons * sizeof(ButtonData));

	int pass = 0;
	for (i = 0; i < numButtons; i++) {
		btsData[i].button = &buttons[i];
		btsData[i].gc = &buttonGC;
		XRectangle btTextDim;
		Xutf8TextExtents(fs, btsData[i].button->label, (int)strlen(btsData[i].button->label),
			&btTextDim, NULL);
		btsData[i].rect.width = (btTextDim.width < dim.btMinWidth) ? dim.btMinWidth :
			(btTextDim.width + 2 * dim.btLateralPad);
		btsData[i].rect.height = dim.btMinHeight;
		btsData[i].rect.x = winW - dim.pad_left - btsData[i].rect.width - pass;
		btsData[i].rect.y = textH + dim.pad_up + dim.pad_down + ((dim.barHeight - dim.btMinHeight) / 2);
		pass += btsData[i].rect.width + dim.btSpacing;
	}

	setWindowTitle(title, &ui.win, ui.dpy);
	XFlush(ui.dpy);

	bool quit = false;
	int res = -1;

	while (!quit) {
		XEvent e;
		XNextEvent(ui.dpy, &e);
		switch (e.type) {
			case MotionNotify:
			case ButtonPress:
			case ButtonRelease:
				for (i = 0; i < numButtons; i++) {
					btsData[i].gc = &buttonGC;
					if (isInside(e.xmotion.x, e.xmotion.y, btsData[i].rect)) {
						btsData[i].gc = &buttonGC_underPointer;
						if (e.type == ButtonPress && e.xbutton.button == Button1) {
							btsData[i].gc = &buttonGC_onClick;
							res = i + 1;
							quit = true;
						}
					}
				}
			case Expose:
				// draw the text in multiply lines
				for (i = 0; i < textLines; i++) {
					Xutf8DrawString(ui.dpy, ui.win, fs, ui.gc, dim.pad_left, dim.pad_up + i * (dim.lineSpacing + 18),
						text_splitted[i], (int)strlen(text_splitted[i]));
				}

				XFillRectangle(ui.dpy, ui.win, barGC, 0, textH + dim.pad_up + dim.pad_down, winW, dim.barHeight);
				for (i = 0; i < numButtons; i++) {
					XFillRectangle(ui.dpy, ui.win, *btsData[i].gc, btsData[i].rect.x, btsData[i].rect.y,
						btsData[i].rect.width, btsData[i].rect.height);

					XRectangle btTextDim;
					Xutf8TextExtents(fs, btsData[i].button->label, (int)strlen(btsData[i].button->label),
						&btTextDim, NULL);
					Xutf8DrawString(ui.dpy, ui.win, fs, ui.gc,
						btsData[i].rect.x + (btsData[i].rect.width - btTextDim.width) / 2,
						btsData[i].rect.y + (btsData[i].rect.height + btTextDim.height) / 2,
						btsData[i].button->label, (int)strlen(btsData[i].button->label));
				}
				XFlush(ui.dpy);
				break;
			case ClientMessage:
				break;
			default:
				break;
		}
	}

	for (i = 0; i < textLines; i++) {
		free(text_splitted[i]);
	}
	free(text_splitted);
	free(btsData);
	if (missingCharset_list)
		XFreeStringList(missingCharset_list);
	XDestroyWindow(ui.dpy, ui.win);
	XFreeFontSet(ui.dpy, fs);
	XFreeGC(ui.dpy, ui.gc);
	XFreeGC(ui.dpy, barGC);
	XFreeGC(ui.dpy, buttonGC);
	XFreeGC(ui.dpy, buttonGC_underPointer);
	XFreeGC(ui.dpy, buttonGC_onClick);
	XFreeColormap(ui.dpy, cmap);
	XCloseDisplay(ui.dpy);

	return res;
}

// clang-format off
static int _GUI_KEYCODES[124] = {XK_BackSpace,8,XK_Delete,127,XK_Down,18,XK_End,5,XK_Escape,27,XK_Home,2,XK_Insert,26,XK_Left,20,XK_Page_Down,4,XK_Page_Up,3,XK_Return,10,XK_Right,19,XK_Tab,9,XK_Up,17,XK_apostrophe,39,XK_backslash,92,XK_bracketleft,91,XK_bracketright,93,XK_comma,44,XK_equal,61,XK_grave,96,XK_minus,45,XK_period,46,XK_semicolon,59,XK_slash,47,XK_space,32,XK_a,65,XK_b,66,XK_c,67,XK_d,68,XK_e,69,XK_f,70,XK_g,71,XK_h,72,XK_i,73,XK_j,74,XK_k,75,XK_l,76,XK_m,77,XK_n,78,XK_o,79,XK_p,80,XK_q,81,XK_r,82,XK_s,83,XK_t,84,XK_u,85,XK_v,86,XK_w,87,XK_x,88,XK_y,89,XK_z,90,XK_0,48,XK_1,49,XK_2,50,XK_3,51,XK_4,52,XK_5,53,XK_6,54,XK_7,55,XK_8,56,XK_9,57};

int gui_window(gui_info *ui, const char *title, int width, int height, int buffered) {
	GLint att[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE,     24,
		GLX_STENCIL_SIZE,   8,
		GLX_RED_SIZE,       8,
		GLX_GREEN_SIZE,     8,
		GLX_BLUE_SIZE,      8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES,        0,
		None
	};
	int argc = 0;
	char **argv = NULL;

	ui->title = title;
	ui->width = (int)width;
	ui->height = (int)height;
	if (buffered == true) {
		ui->buf = malloc(ui->width * ui->height * sizeof(uint32_t));
		if (!ui->buf)
			return 0;
	}

	if (main_gui_info == NULL) {
		ui->wnd = XtOpenApplication(&ui->app_con, ui->title, NULL, 0, &argc, argv,
			fallback, sessionShellWidgetClass, NULL, 0);
		main_gui_info = ui;
	}

	ui->dpy = XtDisplayOfObject(main_gui_info->wnd);
	if (ui->dpy == NULL) {
		XtAppError(ui->app_con, "\n\tcannot connect to X server\n\n");
		return 0;
	}

	ui->screen = DefaultScreen(ui->dpy);
	if (!ui->buf) {
		ui->vi = glXChooseVisual(ui->dpy, ui->screen, att);
		if (ui->vi == NULL) {
			XtAppError(ui->app_con, "\n\tno appropriate visual found\n\n");
			return 0;
		}
	}

	ui->root = RootWindow(ui->dpy, ui->screen);
	ui->win = XCreateSimpleWindow(ui->dpy, ui->root, 0, 0, ui->width, ui->height, 0,
		BlackPixel(ui->dpy, ui->screen), WhitePixel(ui->dpy, ui->screen));

	if (ui->buf)
		ui->gc = XCreateGC(ui->dpy, ui->win, 0, 0);

	XSelectInput(ui->dpy, ui->win, ExposureMask | KeyPressMask | KeyReleaseMask
		| ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
	ui->wmDeleteMessage = XInternAtom(ui->dpy, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(ui->dpy, ui->win, &ui->wmDeleteMessage, 1);

	if (buffered == -1) {
		XSizeHints hints;
		hints.flags = PSize | PMinSize | PMaxSize;
		hints.min_width = hints.max_width = hints.base_width = width;
		hints.min_height = hints.max_height = hints.base_height = height;
		XSetWMNormalHints(ui->dpy, ui->win, &hints);
	}

	gui_seticon(ui, icon_32x32);
	XStoreName(ui->dpy, ui->win, ui->title);

	if (!ui->buf) {
		XSync(ui->dpy, 0);
		ui->glc = glXCreateContext(ui->dpy, ui->vi, NULL, GL_TRUE);
		glXMakeCurrent(ui->dpy, ui->win, ui->glc);
		ui->topLevel = XtAppCreateShell(ui->title, NULL, applicationShellWidgetClass, ui->dpy, NULL, 0);
		XtResizeWidget(ui->topLevel, ui->width, ui->height, 1);
		ui->app->wnd = ui->topLevel;
	} else {
		XSync(ui->dpy, ui->win);
		ui->img = XCreateImage(ui->dpy, DefaultVisual(ui->dpy, 0), 24, ZPixmap, 0,
			(char *)ui->buf, ui->width, ui->height, 32, 0);
	}

	ui->font = XLoadQueryFont(ui->dpy, "lucidasans-8");
	return 1;
}

#include "webview-x11.c"

FORCEINLINE void gui_close(gui_info *ui) {
	gui_free(ui);
}

FORCEINLINE void gui_destroy(gui_info ui) {
	XtDestroyApplicationContext(ui.app_con);
}

int gui_loop(gui_info *ui) {
	//XEvent ev;
	unsigned int i;
	XPutImage(ui->dpy, ui->win, ui->gc, ui->img, 0, 0, 0, 0, ui->width, ui->height);
	XFlush(ui->dpy);
	while (XPending(ui->dpy)) {
		XNextEvent(ui->dpy, &ui->xev);
		switch (ui->xev.type) {
			case ClientMessage:
				if (ui->xev.xclient.data.l[0] == ui->wmDeleteMessage)
					return -(ClientMessage);
				break;
			case ButtonPress:
			case ButtonRelease:
				ui->mouse = (ui->xev.type == ButtonPress);
				break;
			case MotionNotify:
				ui->x = ui->xev.xmotion.x, ui->y = ui->xev.xmotion.y;
				break;
			case KeyPress:
			case KeyRelease: {
					int m = ui->xev.xkey.state;
					int k = XkbKeycodeToKeysym(ui->dpy, ui->xev.xkey.keycode, 0, 0);
				for (i = 0; i < 124; i += 2) {
					if (_GUI_KEYCODES[i] == k) {
						ui->keys[_GUI_KEYCODES[i + 1]] = (ui->xev.type == KeyPress);
						break;
					}
				}
				ui->mod = (!!(m & ControlMask)) | (!!(m & ShiftMask) << 1)
					| (!!(m & Mod1Mask) << 2) | (!!(m & Mod4Mask) << 3);
			} break;
		}
	}
	return 0;
}
#endif

#ifdef _WIN32
void gui_sleep(int64_t ms) { Sleep(ms); }
int64_t gui_time() {
	LARGE_INTEGER freq, count;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&count);
	return (int64_t)(count.QuadPart * 1000.0 / freq.QuadPart);
}
#else
void gui_sleep(int64_t ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
}

int64_t gui_time(void) {
	struct timespec time;
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_sec * 1000 + (time.tv_nsec / 1000000);
}
#endif

FORCEINLINE int gui_webview(gui_info *ui, const char *title, const char *url,
	int width, int height, bool showtoolbar) {
#if !defined(__APPLE__) || !defined(_WIN32)
	if (tls_init() != 0) {
		webview_debug("tls_init failed");
		return 0;
	}
#endif

	ui->web->url = url;
	ui->web->title = title;
	ui->web->width = width;
	ui->web->height = height;
	ui->web->resizable = 1;
	ui->web->showtoolbar = showtoolbar;
	ui->web->debug = 1;
	if (!main_gui_info->bar_info) {
		ui->web->width = main_gui_info->width;
		ui->web->height = main_gui_info->height;
	}

#if defined(__APPLE__)
	if (!main_gui_info->bar_info) {
		cocoa_set_with(main_gui_info->wnd, "setTitle:", (id)cocoa_str(main_gui_info->title));
		main_gui_info->window[0] = main_gui_info->wnd;
	}

	ui->webView[0] = main_gui_info->webView[0];
	ui->window[0] = main_gui_info->window[0];
	ui->delegate = main_gui_info->delegate;
	main_gui_info->web->title = title;
#endif
	return webview_create(ui, ui->web);
}

FORCEINLINE void gui_webactive(gui_info ui) {
#if defined(__APPLE__) || !defined(_WIN32)
	webview_loop(ui.web, 1);
#else
	ui.is_webview = 1;
	ui.wnd = ui.web->priv.hwnd;
	gui_handler(&ui);
	ui.is_webview = 0;
#endif
}

FORCEINLINE void gui_webdestroy(gui_info ui) {
	webview_exit(ui.web);
}

WEBVIEW_API int webview_run(const char *title, const char *url, int width, int height) {
	gui_info ui = {0};
	gui_webview(&ui, title, url, width, height, true);
	gui_webactive(ui);
	gui_webdestroy(ui);
	return 0;
}

WEBVIEW_API void webview_debug(const char *format, ...) {
#ifdef USE_DEBUG
	char buf[4096];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	webview_print_log(buf);
	va_end(ap);
#endif
}

static int webview_js_encode(const char *s, char *esc, size_t n) {
	int r = 1; /* At least one byte for trailing zero */
	for (; *s; s++) {
		const unsigned char c = *s;
		if (c >= 0x20 && c < 0x80 && strchr("<>\\'\"", c) == NULL) {
			if (n > 0) {
				*esc++ = c;
				n--;
			}
			r++;
		} else {
			if (n > 0) {
				snprintf(esc, n, "\\x%02x", (int)c);
				esc += 4;
				n -= 4;
			}
			r += 4;
		}
	}
	return r;
}

WEBVIEW_API int webview_inject_css(webview_t *w, const char *css) {
	int n = webview_js_encode(css, NULL, 0);
	char *esc = (char *)calloc(1, sizeof(CSS_INJECT_FUNCTION) + n + 4);
	if (esc == NULL) {
		return -1;
	}
	char *js = (char *)calloc(1, n);
	webview_js_encode(css, js, n);
	snprintf(esc, sizeof(CSS_INJECT_FUNCTION) + n + 4, "%s(\"%s\")",
		CSS_INJECT_FUNCTION, js);
	int r = webview_eval(w, esc);
	free(js);
	free(esc);
	return r;
}

FORCEINLINE void webview_print_log(const char *s) {
#ifdef USE_DEBUG
	fprintf(stderr, "%s\n", s);
#endif
}