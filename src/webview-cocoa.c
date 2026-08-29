/*
 * MIT License
 *
 * Copyright (c) 2017 Serge Zaitsev
 * Copyright (c) 2023 Badr Ghanem
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
 *
 * Badr Ghanem made modifications on 03/12/2023 to support Mac OS.
 */

#if defined(__APPLE__)
#include <gui.h>
#define NSWindowStyleMaskFullScreen (1 << 14)
#define WKNavigationActionPolicyDownload 2
#define WKNavigationResponsePolicyAllow 1
static id delegate = nil;

static const char *webview_check_url(const char *url) {
	if (url == NULL || strlen(url) == 0) {
		return DEFAULT_URL;
	}
	return url;
}

static void run_open_panel(id self, SEL cmd, id webView,
	id parameters, id frame, void(^completionHandler)(id)) {
	id openPanel = cocoa_get("NSOpenPanel", "openPanel");

	cocoa_set_with(openPanel, "setAllowsMultipleSelection:",
		cocoa_send(parameters, "allowsMultipleSelection"));
	cocoa_set(openPanel, "setCanChooseFiles:", 1);

	cocoa_postimp_func(	openPanel, sel_getUid("beginWithCompletionHandler:"), (IMP)^(id result) {
		if (result == (id)NSModalResponseOK) {
			id urls = cocoa_send(openPanel, "URLs");
			completionHandler(urls);
		} else {
			completionHandler(nil);
		}
	});

}

static void run_save_panel(id self, SEL cmd, id download, id filename,
	void(^completionHandler)(int allowOverwrite, id destination)) {
	id savePanel = cocoa_get("NSSavePanel", "savePanel");
	cocoa_set(savePanel, "setCanCreateDirectories:", 1);
	cocoa_set_with(savePanel, "setNameFieldStringValue:", filename);

	cocoa_postimp_func(savePanel, sel_getUid("beginWithCompletionHandler:"), (IMP)^(id result) {
		if (result == (id)NSModalResponseOK) {
			id url = cocoa_send(savePanel, "URL");
			id path = cocoa_send(url, "path");
			completionHandler(1, path);
		} else {
			completionHandler(NO, nil);
		}
	});
}

static void run_confirmation_panel(id self, SEL cmd, id webView, id message,
	id frame, void(^completionHandler)(bool)) {
	id alert = cocoa_new("NSAlert");
	cocoa_set_with(alert, "setIcon:",
		cocoa_get_with("NSImage", "imageNamed:", (id)cocoa_str("NSCaution")));

	cocoa_set(alert, "setShowsHelp:", 0);
	cocoa_set_with(alert, "setInformativeText:", message);
	cocoa_set_with(alert, "addButtonWithTitle:", (id)cocoa_str("OK"));
	cocoa_set_with(alert, "addButtonWithTitle:", (id)cocoa_str("Cancel"));
	if (((id(*)(id, SEL))objc_msgSend)(alert, sel_getUid("runModal")) ==
		(id)NSAlertFirstButtonReturn) {
		completionHandler(true);
	} else {
		completionHandler(false);
	}

	cocoa_select(alert, "release");
}

static void run_alert_panel(id self, SEL cmd, id webView, id message, id frame,
	void(^completionHandler)(void)) {
	id alert = cocoa_new("NSAlert");
	cocoa_set_with(alert, "setIcon:",
		cocoa_get_with("NSImage", "imageNamed:", (id)cocoa_str("NSCaution")));

	cocoa_set(alert, "setShowsHelp:", 0);
	cocoa_set_with(alert, "setInformativeText:", message);
	cocoa_set_with(alert, "addButtonWithTitle:", (id)cocoa_str("OK"));

	cocoa_select(alert, "runModal");
	cocoa_select(alert, "release");
	completionHandler();
}

static void download_failed(id self, SEL cmd, id download, id error) {
	fprintf(stderr, "%s", cocoa_tochar((NSString)error));
}

static void make_nav_policy_decision(id self, SEL cmd, id webView, id response,
	void(^decisionHandler)(int)) {
	if (cocoa_status(response, "canShowMIMEType") == 0) {
		decisionHandler(WKNavigationActionPolicyDownload);
	} else {
		decisionHandler(WKNavigationResponsePolicyAllow);
	}
}

static const char *parse_data_URI_content_type(const char *uri, int *comma_index) {
	if (uri == NULL || *uri == '\0' || comma_index == NULL) {
		return NULL; // Handling invalid input
	}

	const char *p = uri;
	char result[256]; // Assumed maximum length of the result

	// Ignore whitespace at the beginning of the string
	while (isspace(*p)) {
		p++;
	}

	int i = 0;
	while (*p != ',' && *p != '\0' && i < 255) {
		if (!isspace(*p)) {
			result[i++] = *p;
		}
		p++;
	}
	result[i] = '\0'; // Ensure the result string is null-terminated

	// If 'data:' is found at the start of the result, remove it
	if (strncmp(result, "data:", 5) == 0) {
		memmove(result, result + 5, strlen(result) - 4); // Remove 'data:' by shifting the rest of the string forward
	} else {
		return NULL;
	}

	*comma_index = (int)(p - uri); // Save the index of the first comma

	return strdup(result); // Return a copy of the result (remember to free the allocated memory later)
}

static char *webview_script =
"window.external = this;"
"invoke = function(arg)"
"	{ webkit.messageHandlers.invoke.postMessage(arg); };"
"function sendLink()"
"	{ window.webkit.messageHandlers.newUrlDetected.postMessage(this.href); }"
"var allLinks = document.links;"
"for (var i = 0; i < allLinks.length; i++)"
"	{ allLinks[i].onmouseover = sendLink; }";

int webview_create(gui_info *ui, webview_t *w) {
	w->priv.pool = cocoa_new("NSAutoreleasePool");
	w->priv.windowDelegate = cocoa_send((id)ui->delegate, "new");

/***
	 _WKDownloadDelegate is an undocumented/private protocol with methods called
	 from WKNavigationDelegate
	 References:
	 https://github.com/WebKit/webkit/blob/master/Source/WebKit/UIProcess/API/Cocoa/_WKDownload.h
	 https://github.com/WebKit/webkit/blob/master/Source/WebKit/UIProcess/API/Cocoa/_WKDownloadDelegate.h
	 https://github.com/WebKit/webkit/blob/master/Tools/TestWebKitAPI/Tests/WebKitCocoa/Download.mm


	class_addMethod(__WKScriptMessageHandler,
		sel_getUid("_download:decideDestinationWithSuggestedFilename:completionHandler:"),
		(IMP)run_save_panel, "v@:@@?");
	class_addMethod(__WKScriptMessageHandler,
		sel_getUid("_download:didFailWithError:"),
		(IMP)download_failed, "v@:@@");

	id downloadDelegate = cocoa_send((id)__WKScriptMessageHandler, "new");
	Class __WKPreferences = objc_allocateClassPair(objc_getClass("WKPreferences"), "__WKPreferences", 0);
	objc_property_attribute_t type = {"T", "c"};
	objc_property_attribute_t ownership = {"N", ""};
	objc_property_attribute_t attrs[] = {type, ownership};
	class_replaceProperty(__WKPreferences, "developerExtrasEnabled", attrs, 2);
	objc_registerClassPair(__WKPreferences);
	id wkPref = cocoa_send((id)__WKPreferences, "new");

	cocoa_set_pair(wkPref, "setValue:forKey:",
		cocoa_get_status("NSNumber", "numberWithBool:", !!w->debug),
		(id)cocoa_str("developerExtrasEnabled"));
	***/

	id webViewConfiguration = cocoa_init(cocoa_alloc("WKWebViewConfiguration"));
	id userContentController = cocoa_new("WKUserContentController");
	cocoa_set_pair(userContentController, "addScriptMessageHandler:name:",
		w->priv.windowDelegate, (id)cocoa_str("invoke"));
	cocoa_set_pair(userContentController, "addScriptMessageHandler:name:",
		w->priv.windowDelegate, (id)cocoa_str("newUrlDetected"));

	id windowExternalScript = cocoa_alloc("WKUserScript");
	cocoa_postwithtwo_func(
		windowExternalScript,
		sel_getUid("initWithSource:injectionTime:forMainFrameOnly:"), (id)cocoa_str(webview_script),
		WKUserScriptInjectionTimeAtDocumentEnd, NO);

	cocoa_set_with(userContentController, "addUserScript:", windowExternalScript);
	cocoa_set_with(webViewConfiguration, "setUserContentController:", userContentController);

	if (!delegate) {
		delegate = (id)AppDelClass;
#ifdef USE_DEBUG
		fprintf(stderr, "[ObjC]\t\t\tCreating WebView\n");
#endif
		if (!ui->window[0]) {
			ui->window[0] = cocoa_window(0, 0, w->width, w->height, (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable), NSBackingStoreBuffered, NO);
			if (!ui->window[0]) {
#ifdef USE_DEBUG
				fprintf(stderr, "[ObjC]\t\t\twebview_create() -> Failed to create NSWindow\n");
#endif
				return false;
			}
		}

		ui->webView[0] = (WKWebView)cocoa_sendview_func(cocoa_autorelease("WKWebView"),
			sel_getUid("initWithFrame:configuration:"),
			CGRectMake(0, 14, w->width, (w->showtoolbar ? w->height - 39 : w->height)),
			webViewConfiguration);

		if (w->showtoolbar) {
			class_addMethod(ui->delegate, sel_registerName("webview_home:"), (IMP)webview_home, "v@:@");
			class_addMethod(ui->delegate, sel_registerName("webview_go_back:"), (IMP)webview_go_back, "v@:@");
			class_addMethod(ui->delegate, sel_registerName("webview_go_forward:"), (IMP)webview_go_forward, "v@:@");
			class_addMethod(ui->delegate, sel_registerName("webview_go_to:"), (IMP)webview_go_to, "v@:@");
			class_addMethod(ui->delegate, sel_registerName("webview_reload:"), (IMP)webview_reload, "v@:@");
			class_addMethod(ui->delegate, sel_registerName("webview_stop:"), (IMP)webview_stop, "v@:@");

			cocoa_button(ui->window[0], "Home", "webview_home:", 4, w->height - 22, 50, NSRoundRectBezelStyle, true);
			cocoa_button(ui->window[0], "<", "webview_go_back:", 56, w->height - 22, 25, NSRoundRectBezelStyle, true);
			cocoa_button(ui->window[0], ">", "webview_go_forward:", 78, w->height - 22, 25, NSRoundRectBezelStyle, true);

			int width = w->width - 162;
			NSTextField field = cocoa_field(ui->window[0], ui->webView[0], "", 105,
				w->height - 22, width, field_url);
			w->userdata = (void *)field;
			cocoa_button(ui->window[0], "Go", "webview_go_to:", width + 110, w->height - 22, 40, NSRoundRectBezelStyle, -1);
		}

		w->statusline = gui_statusline(ui->window[0], ui->webView[0], "", 0, w->width - 12);
		if (!ui->webView[0]) {
#ifdef USE_DEBUG
			fprintf(stderr, "[ObjC]\t\t\twebview_create() -> Failed to create WKWebView\n");
#endif
			return false;
		}

		main_gui_info->window[0] = ui->window[0];
		main_gui_info->webView[0] = ui->webView[0];
		main_gui_info->statusLine = w->statusline;
	}

	w->priv.window = ui->window[0];
	w->priv.webview = ui->webView[0];
	cocoa_set_with(ui->window[0], "setDelegate:", w->priv.windowDelegate);
	cocoa_set_with(ui->webView[0], "setUIDelegate:", userContentController);
	cocoa_set_with(ui->webView[0], "setNavigationDelegate:", w->priv.windowDelegate);
	cocoa_set(ui->webView[0], "setAutoresizesSubviews:", 1);
	cocoa_set(ui->webView[0], "setAutoresizingMask:", (NSViewWidthSizable | NSViewHeightSizable));
	objc_setAssociatedObject(userContentController, "webview", (id)(w),
		OBJC_ASSOCIATION_ASSIGN);
	objc_setAssociatedObject(w->priv.windowDelegate, "webview", (id)(w),
		OBJC_ASSOCIATION_ASSIGN);

	int comma_index;
	const char *MIMEType = parse_data_URI_content_type(w->url, &comma_index);
	if (MIMEType != NULL) {
		id NSString = (id)cocoa_str(w->url + (comma_index + 1));
		id NSData = cocoa_sendint_func(NSString, sel_getUid("dataUsingEncoding:"), NSUTF8StringEncoding);

		((void(*)(id, SEL, id, id, id, void *))objc_msgSend)(ui->webView[0],
			sel_getUid("loadData:MIMEType:characterEncodingName:baseURL:"),
			NSData, (id)cocoa_str(MIMEType), (id)cocoa_str("UTF-8"), NULL);

		free((void *)MIMEType);
	} else {
		id nsURL = cocoa_get_with("NSURL", "URLWithString:", (id)cocoa_str(webview_check_url(w->url)));
		cocoa_set_with(ui->webView[0], "loadRequest:", cocoa_get_with("NSURLRequest", "requestWithURL:", nsURL));
	}

	cocoa_set_with(ui->window[0], "setDelegate:", w->priv.windowDelegate);
	cocoa_set_with(ui->window[0], "setTitle:", (id)cocoa_str(w->title));
	cocoa_set_with(cocoa_content_view(ui->window[0]), "addSubview:", ui->webView[0]);
	cocoa_set_with(ui->window[0], "makeKeyAndOrderFront:", nil);
	cocoa_set(ui->window[0], "setAutorecalculatesKeyViewLoop:", YES);
	cocoa_select(ui->window[0], "center");
	cocoa_set(ui->window[0], "setIsVisible:", YES);
	ui->delegate_instance = nil;
	ui->app->wnd = ui->window[0];
	ui->app->gui = ui;
	ui->app->running = YES;
	w->priv.should_exit = 0;
	main_gui_info->web->priv = w->priv;
	cocoa_set(cocoa_send(w->priv.window, "contentView"), "setNeedsDisplay:", YES);

	return 1;
}

int webview_loop(webview_t *w, int blocking) {
	id event;
	if (blocking) {
		while (!main_gui_info->web->priv.should_exit) {
			event = cocoa_next_event(NSApp, NSUIntegerMax, nil, NSDefaultRunLoopMode, YES);
			if (event) {
				cocoa_set_with(NSApp, "sendEvent:", event);
				cocoa_select(NSApp, "updateWindows");
			}
		}
		w->priv.should_exit = 1;
	} else {
		event = cocoa_next_event(NSApp, NSUIntegerMax, cocoa_get("NSDate", "distantPast"), (id)kCFRunLoopDefaultMode, YES);
		if (event) {
			cocoa_set_with(NSApp, "sendEvent:", event);
		}
	}

	return w->priv.should_exit;
}

int webview_eval(webview_t *w, const char *js) {
	cocoa_set_pair(w->priv.webview, "evaluateJavaScript:completionHandler:", (id)cocoa_str(js), nil);
	return 0;
}

FORCEINLINE void webview_set_title(webview_t *w, const char *title) {
	cocoa_set_with(w->priv.window, "setTitle:",	(id)cocoa_str(title));
}

FORCEINLINE void webview_set_fullscreen(webview_t *w, int fullscreen) {
	unsigned long windowStyleMask = cocoa_status(w->priv.window, "styleMask");
	int b = (((windowStyleMask & NSWindowStyleMaskFullScreen) ==
		NSWindowStyleMaskFullScreen)
		? 1
		: 0);
	if (b != fullscreen) {
		cocoa_postany_func(w->priv.window, sel_getUid("toggleFullScreen:"), NULL);
	}
}

void webview_set_color(webview_t *w, uint8_t r, uint8_t g,
	uint8_t b, uint8_t a) {

	id color = ((id(*)(id, SEL, float, float, float, float))objc_msgSend)((id)objc_getClass("NSColor"),
		sel_getUid("colorWithRed:green:blue:alpha:"),
		(float)r / 255.0, (float)g / 255.0, (float)b / 255.0,
		(float)a / 255.0);

	cocoa_set_with(w->priv.window, "setBackgroundColor:", color);
	if (0.5 >= ((r / 255.0 * 299.0) + (g / 255.0 * 587.0) + (b / 255.0 * 114.0)) /
		1000.0) {
		cocoa_set_with(w->priv.window, "setAppearance:",
			cocoa_get_with("NSAppearance", "appearanceNamed:", (id)cocoa_str("NSAppearanceNameVibrantDark")));
	} else {
		cocoa_set_with(w->priv.window, "setAppearance:",
			cocoa_get_with("NSAppearance", "appearanceNamed:", (id)cocoa_str("NSAppearanceNameVibrantLight")));
	}

	cocoa_set(w->priv.window, "setOpaque:", 0);
	cocoa_set(w->priv.window, "setTitlebarAppearsTransparent:", 1);
}

FORCEINLINE ui_wnd_t webview_get_window(webview_t *w) {
	return w->priv.window;
}

void webview_set_size(webview_t *w, int width, int height, int hints) {
	NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable;
	if (hints != WEBVIEW_HINT_FIXED) {
		style = style | NSWindowStyleMaskResizable;
	}

	cocoa_set(w->priv.window, "setStyleMask:", style);

	if (hints == WEBVIEW_HINT_MIN) {
		cocoa_set_size(w->priv.window, "setContentMinSize:", width, height);
	} else if (hints == WEBVIEW_HINT_MAX) {
		cocoa_set_size(w->priv.window, "setContentMaxSize:", width, height);
	} else {
		((void (*)(id, SEL, CGRect, BOOL, BOOL))objc_msgSend)(
			w->priv.window, sel_getUid("setFrame:display:animate:"),
			CGRectMake(0, 0, width, height), 1, 0);
	}

	cocoa_select(w->priv.window, "center");
}

FORCEINLINE void webview_navigate(webview_t *w, const char *url) {
	id nsURL = cocoa_get_with("NSURL", "URLWithString:", (id)cocoa_str(url));
	cocoa_set_with(w->priv.webview, "loadRequest:", cocoa_get_with("NSURLRequest", "requestWithURL:", nsURL));
}

FORCEINLINE void webview_go_to(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w) {
		NSTextField url = (NSTextField)w->userdata;
		ui_field_str(curl, url);
		if (is_ValidUrl(curl)) {
			cocoa_set_with(w->priv.webview, "loadRequest:",
				cocoa_get_with("NSURLRequest", "requestWithURL:",
					cocoa_get_with("NSURL", "URLWithString:", (id)curl)));
		}
	}
}

FORCEINLINE void webview_home(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w) {
		id nsURL = cocoa_get_with("NSURL", "URLWithString:", (id)cocoa_str(w->url));
		cocoa_set_with(w->priv.webview, "loadRequest:", cocoa_get_with("NSURLRequest", "requestWithURL:", nsURL));
	}
}

void webview_loadfile(webview_t *w, const char *resourcefile, const char *type) {
	NSBundle mainBundle = cocoa_get("NSBundle", "mainBundle");
	NSString filePath = (NSString)cocoa_sendpair_func(mainBundle, sel_getUid("pathForResource:ofType:"),
		(id)cocoa_str(resourcefile), (id)cocoa_str(type));

	NSURL url = (NSURL)cocoa_get_with("NSURL", "fileURLWithPath:", (id)filePath);

	cocoa_set_pair(w->priv.webview, "loadFileURL:allowingReadAccessToURL:", (id)url,
		cocoa_send((id)url, "URLByDeletingLastPathComponent"));
}

FORCEINLINE void webview_set_html(webview_t *w, const char *html) {
	cocoa_set_pair(w->priv.webview, "loadHTMLString:baseURL:", (id)cocoa_str(html), nil);
}

FORCEINLINE void webview_go_back(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w && (bool)cocoa_status(w->priv.webview, "canGoBack"))
		cocoa_select(w->priv.webview, "goBack");
}

FORCEINLINE void webview_go_forward(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w && (bool)cocoa_status(w->priv.webview, "canGoForward"))
		cocoa_select(w->priv.webview, "goForward");
}

FORCEINLINE void webview_reload(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w)
		cocoa_select(w->priv.webview, "reload");
}

FORCEINLINE void webview_stop(__GUI_FIELD__) {
	webview_t *w = (webview_t *)objc_getAssociatedObject(self, "webview");
	if (w)
		cocoa_select(w->priv.webview, "stopLoading");
}

FORCEINLINE char *webview_get_title(webview_t *w) {
	return cocoa_tochar((NSString)cocoa_send(w->priv.webview, "title"));
}

FORCEINLINE char *webview_get_url(webview_t *w) {
	return cocoa_tochar((NSString)cocoa_send(w->priv.webview, "url"));
}

void webview_dialog(webview_t *w,
	enum webview_dialog_type dlgtype, int flags,
	const char *title, const char *arg,
	char *result, size_t resultsz) {
	if (dlgtype == WEBVIEW_DIALOG_TYPE_OPEN ||
		dlgtype == WEBVIEW_DIALOG_TYPE_SAVE) {
		id panel = (id)objc_getClass("NSSavePanel");
		if (dlgtype == WEBVIEW_DIALOG_TYPE_OPEN) {
			id openPanel = cocoa_get("NSOpenPanel", "openPanel");
			if (flags & WEBVIEW_DIALOG_FLAG_DIRECTORY) {
				cocoa_set(openPanel, "setCanChooseFiles:", 0);
				cocoa_set(openPanel, "setCanChooseDirectories:", 1);
			} else {
				cocoa_set(openPanel, "setCanChooseFiles:", 1);
				cocoa_set(openPanel, "setCanChooseDirectories:", 0);
			}

			cocoa_set(openPanel, "setResolvesAliases:", 0);
			cocoa_set(openPanel, "setAllowsMultipleSelection:",	0);
			panel = openPanel;
		} else {
			panel = cocoa_get("NSSavePanel", "savePanel");
		}

		cocoa_set(panel, "setCanCreateDirectories:", 1);
		cocoa_set(panel, "setShowsHiddenFiles:", 1);
		cocoa_set(panel, "setExtensionHidden:", 0);
		cocoa_set(panel, "setCanSelectHiddenExtension:", 0);
		cocoa_set(panel, "setTreatsFilePackagesAsDirectories:", 1);
		cocoa_model_func(panel, sel_getUid("beginSheetModalForWindow:completionHandler:"),
			w->priv.window, (IMP)^(id result) {
			cocoa_set_with(NSApp, "stopModalWithCode:", result);
		});

		if (cocoa_send_with(NSApp, "runModalForWindow:", panel) == (id)NSModalResponseOK) {
			id url = cocoa_send(panel, "URL");
			id path = cocoa_send(url, "path");
			const char *filename = cocoa_tochar((NSString)path);
			strlcpy(result, filename, resultsz);
		}
	} else if (dlgtype == WEBVIEW_DIALOG_TYPE_ALERT) {
		id a = cocoa_new("NSAlert");
		switch (flags & WEBVIEW_DIALOG_FLAG_ALERT_MASK) {
			case WEBVIEW_DIALOG_FLAG_INFO:
				cocoa_set(a, "setAlertStyle:", NSAlertStyleInformational);
				break;
			case WEBVIEW_DIALOG_FLAG_WARNING:
				fprintf(stderr, "Warning\n");
				cocoa_set(a, "setAlertStyle:", NSAlertStyleWarning);
				break;
			case WEBVIEW_DIALOG_FLAG_ERROR:
				fprintf(stderr, "Error\n");
				cocoa_set(a, "setAlertStyle:", NSAlertStyleCritical);
				break;
		}

		cocoa_set(a, "setShowsHelp:", 0);
		cocoa_set(a, "setShowsSuppressionButton:", 0);
		cocoa_set_with(a, "setMessageText:", (id)cocoa_str(title));
		cocoa_set_with(a, "setInformativeText:", (id)cocoa_str(arg));
		cocoa_set_with(a, "addButtonWithTitle:", (id)cocoa_str("OK"));
		cocoa_select(a, "runModal");
		cocoa_select(a, "release");
	}
}

static void webview_dispatch_cb(void *arg) {
	struct webview_dispatch_arg *context = (struct webview_dispatch_arg *)arg;
	(context->fn)(context->w, context->arg);
	free(context);
}

void webview_dispatch(webview_t *w, webview_dispatch_fn fn,
	void *arg) {
	struct webview_dispatch_arg *context = (struct webview_dispatch_arg *)malloc(
		sizeof(struct webview_dispatch_arg));
	context->w = w;
	context->arg = arg;
	context->fn = fn;
	dispatch_async_f(dispatch_get_main_queue(), context, webview_dispatch_cb);
}

void webview_exit(webview_t *w) {
	if (main_gui_info->web->priv.webview == w->priv.webview)
		memset((void *)main_gui_info->web, 0, sizeof(main_gui_info->web));

	cocoa_select(w->priv.pool, "drain");
	object_dispose(w->priv.windowDelegate);
}

FORCEINLINE void webview_print_log(const char *s) { fprintf(stderr, "%s\n", s); }
#endif