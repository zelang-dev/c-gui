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
#define WKUserScriptInjectionTimeAtDocumentStart 0
static id delegate = nil;

static const char *webview_check_url(const char *url) {
	if (url == NULL || strlen(url) == 0) {
		return DEFAULT_URL;
	}
	return url;
}

static void webview_window_will_close(id self, SEL cmd, id notification) {
	struct webview *w = (struct webview *)objc_getAssociatedObject(self, "webview");
	if (w)
		webview_terminate(w);
}

static void webview_external_invoke(id self, SEL cmd, id contentController,	id message) {
	struct webview *w = (struct webview *)objc_getAssociatedObject(contentController, "webview");
	if (w == NULL || w->external_invoke_cb == NULL) {
		return;
	}

	w->external_invoke_cb(w, ((const char *(*)(id, SEL))objc_msgSend)(
		((id(*)(id, SEL))objc_msgSend)(message, sel_getUid("body")),
		sel_getUid("UTF8String")));
}

static void run_open_panel(id self, SEL cmd, id webView,
	id parameters, id frame, void(^completionHandler)(id)) {
	id openPanel = cocoa_get("NSOpenPanel", "openPanel");

	cocoa_set_with(openPanel, "setAllowsMultipleSelection:",
		cocoa_send(parameters, "allowsMultipleSelection"));
	cocoa_set(openPanel, "setCanChooseFiles:", 1);

	((id(*)(id, SEL, void(^)(id)))objc_msgSend)(
		openPanel, sel_getUid("beginWithCompletionHandler:"), ^(id result) {
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
	((void(*)(id, SEL, void(^)(id)))objc_msgSend)(savePanel, sel_getUid("beginWithCompletionHandler:"),
		^(id result) {
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
	cocoa_set_with(alert, "setIcon:", cocoa_get_with("NSImage", "imageNamed:", (id)cocoa_str("NSCaution")));

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

	((void(*)(id, SEL, id))objc_msgSend)(alert, sel_getUid("setIcon:"),
		((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSImage"),
			sel_getUid("imageNamed:"),
			(id)cocoa_str("NSCaution")));

	((void(*)(id, SEL, int))objc_msgSend)(alert, sel_getUid("setShowsHelp:"), 0);
	((void(*)(id, SEL, id))objc_msgSend)(alert, sel_getUid("setInformativeText:"), message);
	((void(*)(id, SEL, id))objc_msgSend)(alert, sel_getUid("addButtonWithTitle:"),
		(id)cocoa_str("OK"));
	((void(*)(id, SEL))objc_msgSend)(alert, sel_getUid("runModal"));
	((void(*)(id, SEL))objc_msgSend)(alert, sel_getUid("release"));
	completionHandler();
}

static void download_failed(id self, SEL cmd, id download, id error) {
	fprintf(stderr, "%s", cocoa_tochar((NSString)error));
}

static void make_nav_policy_decision(id self, SEL cmd, id webView, id response,
	void(^decisionHandler)(int)) {
	if (((id(*)(id, SEL))objc_msgSend)(response, sel_getUid("canShowMIMEType")) == 0) {
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

int webview_create(gui_info *ui, webview_t *w) {
	w->priv.pool = cocoa_new("NSAutoreleasePool");
	/*
	Class __WKScriptMessageHandler = objc_allocateClassPair(
		objc_getClass("NSObject"), "__WKScriptMessageHandler", 0);
	class_addMethod(
		__WKScriptMessageHandler,
		sel_getUid("userContentController:didReceiveScriptMessage:"),
		(IMP)webview_external_invoke, "v@:@@");
	objc_registerClassPair(__WKScriptMessageHandler);

	id scriptMessageHandler = cocoa_send((id)__WKScriptMessageHandler, "new");
	*/
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

	id userController = cocoa_new("WKUserContentController");
	objc_setAssociatedObject(userController, "webview", (id)(w), OBJC_ASSOCIATION_ASSIGN);
	cocoa_set_pair(userController, "addScriptMessageHandler:name:",
		scriptMessageHandler, (id)cocoa_str("invoke"));
	***/

	/***
	 In order to maintain compatibility with the other 'webviews' we need to
	 override window.external.invoke to call
	 webkit.messageHandlers.invoke.postMessage

	id windowExternalOverrideScript = cocoa_alloc("WKUserScript");
	((void(*)(id, SEL, id, int, int))objc_msgSend)(
		windowExternalOverrideScript,
		sel_getUid("initWithSource:injectionTime:forMainFrameOnly:"),
		(id)cocoa_str("window.external = this; invoke = function(arg){ webkit.messageHandlers.invoke.postMessage(arg); };"),
		WKUserScriptInjectionTimeAtDocumentStart,
		0);

	cocoa_set_with(userController, "addUserScript:", windowExternalOverrideScript);
	id config = cocoa_new("WKWebViewConfiguration");

	id processPool = cocoa_send(config, "processPool");
	cocoa_set_with(processPool, "_setDownloadDelegate:", downloadDelegate);
	cocoa_set_with(config, "setProcessPool:", processPool);
	cocoa_set_with(config, "setUserContentController:", userController);
	cocoa_set_with(config, "setPreferences:", wkPref);

	CGRect r = CGRectMake(0, 0, w->width, w->height);
	w->priv.webview = cocoa_alloc("WKWebView");
	(void)cocoa_sendview_func(w->priv.webview,
		sel_getUid("initWithFrame:configuration:"), r, config); ***/

	w->priv.windowDelegate = cocoa_send((id)ui->delegate, "new");
	if (!delegate) {
		delegate = (id)AppDelClass;
#ifdef USE_DEBUG
		fprintf(stderr, "[ObjC]\t\t\tCreating WebView\n");
#endif

		ui->window[0] = cocoa_window(0, 0, w->width, w->height, (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable), NSBackingStoreBuffered, NO);
		if (!ui->window[0]) {
#ifdef USE_DEBUG
			fprintf(stderr, "[ObjC]\t\t\twebview_create() -> Failed to create NSWindow\n");
#endif
			return false;
		}

		CGRect r = CGRectMake(0, 0, w->width, w->height);
		ui->webView[0] = (WKWebView)cocoa_sendrect_func(cocoa_autorelease("WKWebView"), sel_getUid("initWithFrame:"), r);

		if (!ui->webView[0]) {
#ifdef USE_DEBUG
			fprintf(stderr, "[ObjC]\t\t\twebview_create() -> Failed to create WKWebView\n");
#endif
			return false;
		}

		main_gui_info->window[0] = ui->window[0];
		main_gui_info->webView[0] = ui->webView[0];
	}

	w->priv.window = ui->window[0];
	w->priv.webview = ui->webView[0];
	cocoa_set_with(ui->window[0], "setDelegate:", w->priv.windowDelegate);
	cocoa_set_with(ui->webView[0], "setUIDelegate:", w->priv.windowDelegate);
	cocoa_set_with(ui->webView[0], "setNavigationDelegate:", w->priv.windowDelegate);
	cocoa_set(ui->webView[0], "setAutoresizesSubviews:", 1);
	cocoa_set(ui->webView[0], "setAutoresizingMask:", (NSViewWidthSizable | NSViewHeightSizable));
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

	return 1;
}

int webview_loop(struct webview *w, int blocking) {
	id until = (blocking ? ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"),
		sel_getUid("distantFuture"))
		: ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"),
			sel_getUid("distantPast")));

	id event = cocoa_next_event(NSApp, NSUIntegerMax, until, (id)kCFRunLoopDefaultMode, YES);
	if (event) {
		cocoa_set_with(NSApp, "sendEvent:", event);
	}

	return w->priv.should_exit;
}

int webview_eval(struct webview *w, const char *js) {
	((void(*)(id, SEL, id, void *))objc_msgSend)(w->priv.webview,
		sel_getUid("evaluateJavaScript:completionHandler:"),
		(id)cocoa_str(js), NULL);
	return 0;
}

FORCEINLINE void webview_set_title(struct webview *w, const char *title) {
	cocoa_set_with(w->priv.window, "setTitle:",	(id)cocoa_str(title));
}

FORCEINLINE void webview_set_fullscreen(struct webview *w, int fullscreen) {
	unsigned long windowStyleMask = cocoa_status(w->priv.window, "styleMask");
	int b = (((windowStyleMask & NSWindowStyleMaskFullScreen) ==
		NSWindowStyleMaskFullScreen)
		? 1
		: 0);
	if (b != fullscreen) {
		cocoa_postany_func(w->priv.window, sel_getUid("toggleFullScreen:"), NULL);
	}
}

void webview_set_color(struct webview *w, uint8_t r, uint8_t g,
	uint8_t b, uint8_t a) {

	id color = ((id(*)(id, SEL, float, float, float, float))objc_msgSend)((id)objc_getClass("NSColor"),
		sel_getUid("colorWithRed:green:blue:alpha:"),
		(float)r / 255.0, (float)g / 255.0, (float)b / 255.0,
		(float)a / 255.0);

	((void(*)(id, SEL, id))objc_msgSend)(w->priv.window, sel_getUid("setBackgroundColor:"), color);

	if (0.5 >= ((r / 255.0 * 299.0) + (g / 255.0 * 587.0) + (b / 255.0 * 114.0)) /
		1000.0) {
		((void(*)(id, SEL, id))objc_msgSend)(w->priv.window, sel_getUid("setAppearance:"),
			((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSAppearance"),
				sel_getUid("appearanceNamed:"),
				(id)cocoa_str("NSAppearanceNameVibrantDark")));
	} else {
		((void(*)(id, SEL, id))objc_msgSend)(w->priv.window, sel_getUid("setAppearance:"),
			((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSAppearance"),
				sel_getUid("appearanceNamed:"),
				(id)cocoa_str("NSAppearanceNameVibrantLight")));
	}
	((void(*)(id, SEL, int))objc_msgSend)(w->priv.window, sel_getUid("setOpaque:"), 0);
	((void(*)(id, SEL, int))objc_msgSend)(w->priv.window,
		sel_getUid("setTitlebarAppearsTransparent:"), 1);
}

void webview_dialog(struct webview *w,
	enum webview_dialog_type dlgtype, int flags,
	const char *title, const char *arg,
	char *result, size_t resultsz) {
	if (dlgtype == WEBVIEW_DIALOG_TYPE_OPEN ||
		dlgtype == WEBVIEW_DIALOG_TYPE_SAVE) {
		id panel = (id)objc_getClass("NSSavePanel");
		if (dlgtype == WEBVIEW_DIALOG_TYPE_OPEN) {
			id openPanel = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSOpenPanel"),
				sel_getUid("openPanel"));
			if (flags & WEBVIEW_DIALOG_FLAG_DIRECTORY) {
				((void(*)(id, SEL, int))objc_msgSend)(openPanel, sel_getUid("setCanChooseFiles:"), 0);
				((void(*)(id, SEL, int))objc_msgSend)(openPanel, sel_getUid("setCanChooseDirectories:"),
					1);
			} else {
				((void(*)(id, SEL, int))objc_msgSend)(openPanel, sel_getUid("setCanChooseFiles:"), 1);
				((void(*)(id, SEL, int))objc_msgSend)(openPanel, sel_getUid("setCanChooseDirectories:"),
					0);
			}
			((void(*)(id, SEL, int))objc_msgSend)(openPanel, sel_getUid("setResolvesAliases:"), 0);
			((void(*)(id, SEL, int))objc_msgSend)(openPanel, sel_getUid("setAllowsMultipleSelection:"),
				0);
			panel = openPanel;
		} else {
			panel = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSSavePanel"),
				sel_getUid("savePanel"));
		}

		((void(*)(id, SEL, int))objc_msgSend)(panel, sel_getUid("setCanCreateDirectories:"), 1);
		((void(*)(id, SEL, int))objc_msgSend)(panel, sel_getUid("setShowsHiddenFiles:"), 1);
		((void(*)(id, SEL, int))objc_msgSend)(panel, sel_getUid("setExtensionHidden:"), 0);
		((void(*)(id, SEL, int))objc_msgSend)(panel, sel_getUid("setCanSelectHiddenExtension:"), 0);
		((void(*)(id, SEL, int))objc_msgSend)(panel, sel_getUid("setTreatsFilePackagesAsDirectories:"),
			1);
		((void(*)(id, SEL, id, void(^)(id)))objc_msgSend)(
			panel, sel_getUid("beginSheetModalForWindow:completionHandler:"),
			w->priv.window, ^(id result) {
			cocoa_set_with(NSApp, "stopModalWithCode:", result);
		});

		if (((id(*)(id, SEL, id))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
			sel_getUid("sharedApplication")),
			sel_getUid("runModalForWindow:"),
			panel) == (id)NSModalResponseOK) {
			id url = ((id(*)(id, SEL))objc_msgSend)(panel, sel_getUid("URL"));
			id path = ((id(*)(id, SEL))objc_msgSend)(url, sel_getUid("path"));
			const char *filename = ((const char *(*)(id, SEL))objc_msgSend)(path, sel_getUid("UTF8String"));
			strlcpy(result, filename, resultsz);
		}
	} else if (dlgtype == WEBVIEW_DIALOG_TYPE_ALERT) {
		id a = cocoa_new("NSAlert");
		switch (flags & WEBVIEW_DIALOG_FLAG_ALERT_MASK) {
			case WEBVIEW_DIALOG_FLAG_INFO:
				((void(*)(id, SEL, int))objc_msgSend)(a, sel_getUid("setAlertStyle:"),
					NSAlertStyleInformational);
				break;
			case WEBVIEW_DIALOG_FLAG_WARNING:
				printf("Warning\n");
				((void(*)(id, SEL, int))objc_msgSend)(a, sel_getUid("setAlertStyle:"), NSAlertStyleWarning);
				break;
			case WEBVIEW_DIALOG_FLAG_ERROR:
				printf("Error\n");
				((void(*)(id, SEL, int))objc_msgSend)(a, sel_getUid("setAlertStyle:"), NSAlertStyleCritical);
				break;
		}
		((void(*)(id, SEL, int))objc_msgSend)(a, sel_getUid("setShowsHelp:"), 0);
		((void(*)(id, SEL, int))objc_msgSend)(a, sel_getUid("setShowsSuppressionButton:"), 0);
		((void(*)(id, SEL, id))objc_msgSend)(a, sel_getUid("setMessageText:"), (id)cocoa_str(title));
		((void(*)(id, SEL, id))objc_msgSend)(a, sel_getUid("setInformativeText:"), (id)cocoa_str(arg));
		((void(*)(id, SEL, id))objc_msgSend)(a, sel_getUid("addButtonWithTitle:"),
			(id)cocoa_str("OK"));
		((void(*)(id, SEL))objc_msgSend)(a, sel_getUid("runModal"));
		((void(*)(id, SEL))objc_msgSend)(a, sel_getUid("release"));
	}
}

static void webview_dispatch_cb(void *arg) {
	struct webview_dispatch_arg *context = (struct webview_dispatch_arg *)arg;
	(context->fn)(context->w, context->arg);
	free(context);
}

void webview_dispatch(struct webview *w, webview_dispatch_fn fn,
	void *arg) {
	struct webview_dispatch_arg *context = (struct webview_dispatch_arg *)malloc(
		sizeof(struct webview_dispatch_arg));
	context->w = w;
	context->arg = arg;
	context->fn = fn;
	dispatch_async_f(dispatch_get_main_queue(), context, webview_dispatch_cb);
}

FORCEINLINE void webview_terminate(struct webview *w) {
	cocoa_set(w->priv.window, "setIsVisible:", NO);
	w->priv.should_exit = 1;
}

FORCEINLINE void webview_exit(struct webview *w) {
	cocoa_select(w->priv.pool, "drain");
	object_dispose(w->priv.windowDelegate);
	w->priv.webview = nil;
}

FORCEINLINE void webview_print_log(const char *s) { fprintf(stderr, "%s\n", s); }
#endif