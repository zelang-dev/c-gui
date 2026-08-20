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

static const char *webview_check_url(const char *url) {
	if (url == NULL || strlen(url) == 0) {
		return DEFAULT_URL;
	}
	return url;
}

static void webview_window_will_close(id self, SEL cmd, id notification) {
	struct webview *w = (struct webview *)objc_getAssociatedObject(self, "webview");
	webview_terminate(w);
}

static void webview_external_invoke(id self, SEL cmd, id contentController,
	id message) {
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
	printf("%s", cocoa_tochar((NSString)error));
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
	Class __WKScriptMessageHandler = objc_allocateClassPair(
		objc_getClass("NSObject"), "__WKScriptMessageHandler", 0);
	class_addMethod(
		__WKScriptMessageHandler,
		sel_getUid("userContentController:didReceiveScriptMessage:"),
		(IMP)webview_external_invoke, "v@:@@");
	objc_registerClassPair(__WKScriptMessageHandler);

	id scriptMessageHandler = cocoa_send((id)__WKScriptMessageHandler, "new");

	/***
	 _WKDownloadDelegate is an undocumented/private protocol with methods called
	 from WKNavigationDelegate
	 References:
	 https://github.com/WebKit/webkit/blob/master/Source/WebKit/UIProcess/API/Cocoa/_WKDownload.h
	 https://github.com/WebKit/webkit/blob/master/Source/WebKit/UIProcess/API/Cocoa/_WKDownloadDelegate.h
	 https://github.com/WebKit/webkit/blob/master/Tools/TestWebKitAPI/Tests/WebKitCocoa/Download.mm
	 ***/

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

	/***
	 In order to maintain compatibility with the other 'webviews' we need to
	 override window.external.invoke to call
	 webkit.messageHandlers.invoke.postMessage  ***/

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

	Class __NSWindowDelegate = objc_allocateClassPair(objc_getClass("NSObject"),
		"__NSWindowDelegate", 0);
	class_addProtocol(__NSWindowDelegate, objc_getProtocol("NSWindowDelegate"));
	class_replaceMethod(__NSWindowDelegate, sel_getUid("windowWillClose:"),
		(IMP)webview_window_will_close, "v@:@");
	objc_registerClassPair(__NSWindowDelegate);

	w->priv.windowDelegate = cocoa_send((id)__NSWindowDelegate, "new");
	objc_setAssociatedObject(w->priv.windowDelegate, "webview", (id)(w),
		OBJC_ASSOCIATION_ASSIGN);

	CGRect r = CGRectMake(0, 0, w->width, w->height);
	unsigned int style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable;
	if (w->resizable) {
		style = style | NSWindowStyleMaskResizable;
	}

	w->priv.window = cocoa_init_window(0, 0, w->width, w->height, style, NSBackingStoreBuffered, YES);
	cocoa_select(w->priv.window, "autorelease");
	cocoa_set_with(w->priv.window, "setDelegate:", w->priv.windowDelegate);
	cocoa_set_with(w->priv.window, "setTitle:", (id)cocoa_str(w->title));
	cocoa_select(w->priv.window, "center");

	Class __WKUIDelegate = objc_allocateClassPair(objc_getClass("NSObject"), "__WKUIDelegate", 0);
	class_addProtocol(__WKUIDelegate, objc_getProtocol("WKUIDelegate"));
	class_addMethod(__WKUIDelegate,
		sel_getUid("webView:runOpenPanelWithParameters:"
			"initiatedByFrame:completionHandler:"),
		(IMP)run_open_panel, "v@:@@@?");
	class_addMethod(__WKUIDelegate,
		sel_getUid("webView:runJavaScriptAlertPanelWithMessage:"
			"initiatedByFrame:completionHandler:"),
		(IMP)run_alert_panel, "v@:@@@?");
	class_addMethod(__WKUIDelegate,
		sel_getUid("webView:runJavaScriptConfirmPanelWithMessage:"
			"initiatedByFrame:completionHandler:"),
		(IMP)run_confirmation_panel, "v@:@@@?");
	objc_registerClassPair(__WKUIDelegate);
	id uiDel = cocoa_send((id)__WKUIDelegate, "new");

	Class __WKNavigationDelegate = objc_allocateClassPair(
		objc_getClass("NSObject"), "__WKNavigationDelegate", 0);
	class_addProtocol(__WKNavigationDelegate,
		objc_getProtocol("WKNavigationDelegate"));
	class_addMethod(
		__WKNavigationDelegate,
		sel_getUid(
			"webView:decidePolicyForNavigationResponse:decisionHandler:"),
		(IMP)make_nav_policy_decision, "v@:@@?");

	objc_registerClassPair(__WKNavigationDelegate);
	id navDel = cocoa_send((id)__WKNavigationDelegate, "new");

	w->priv.webview = cocoa_alloc("WKWebView");
	(void)cocoa_sendview_func(w->priv.webview,
		sel_getUid("initWithFrame:configuration:"), r, config);

	cocoa_set_with(w->priv.webview, "setUIDelegate:", uiDel);
	cocoa_set_with(w->priv.webview, "setNavigationDelegate:", navDel);

	int comma_index;

	const char *MIMEType = parse_data_URI_content_type(w->url, &comma_index);

	if (MIMEType != NULL) {
		id NSString = (id)cocoa_str(w->url + (comma_index + 1));
		id NSData = ((id(*)(id, SEL, int))objc_msgSend)(NSString, sel_getUid("dataUsingEncoding:"), NSUTF8StringEncoding);

		((void(*)(id, SEL, id, id, id, void *))objc_msgSend)(w->priv.webview,
			sel_getUid("loadData:MIMEType:characterEncodingName:baseURL:"),
			NSData, (id)cocoa_str(MIMEType), (id)cocoa_str("UTF-8"), NULL);

		free((void *)MIMEType);
	} else {
		id nsURL = cocoa_get_with("NSURL", "URLWithString:", (id)cocoa_str(webview_check_url(w->url)));
		cocoa_set_with(w->priv.webview, "loadRequest:", cocoa_get_with("NSURLRequest", "requestWithURL:", nsURL));
	}

	cocoa_set(w->priv.webview, "setAutoresizesSubviews:", 1);
	cocoa_set(w->priv.webview, "setAutoresizingMask:", (NSViewWidthSizable | NSViewHeightSizable));
	cocoa_set_with(cocoa_content_view(w->priv.window), "addSubview:", w->priv.webview);
	cocoa_select(w->priv.window, "orderFrontRegardless");

	w->priv.should_exit = 0;
	return 0;
}

int webview_init(struct webview *w) {
	w->priv.pool = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSAutoreleasePool"),
		sel_registerName("new"));
	((void(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
		sel_registerName("sharedApplication"));

	Class __WKScriptMessageHandler = objc_allocateClassPair(
		objc_getClass("NSObject"), "__WKScriptMessageHandler", 0);
	class_addMethod(
		__WKScriptMessageHandler,
		sel_registerName("userContentController:didReceiveScriptMessage:"),
		(IMP)webview_external_invoke, "v@:@@");
	objc_registerClassPair(__WKScriptMessageHandler);

	id scriptMessageHandler =
		((id(*)(id, SEL))objc_msgSend)((id)__WKScriptMessageHandler, sel_registerName("new"));

	/***
	 _WKDownloadDelegate is an undocumented/private protocol with methods called
	 from WKNavigationDelegate
	 References:
	 https://github.com/WebKit/webkit/blob/master/Source/WebKit/UIProcess/API/Cocoa/_WKDownload.h
	 https://github.com/WebKit/webkit/blob/master/Source/WebKit/UIProcess/API/Cocoa/_WKDownloadDelegate.h
	 https://github.com/WebKit/webkit/blob/master/Tools/TestWebKitAPI/Tests/WebKitCocoa/Download.mm
	 ***/

	Class __WKDownloadDelegate = objc_allocateClassPair(
		objc_getClass("NSObject"), "__WKDownloadDelegate", 0);
	class_addMethod(
		__WKDownloadDelegate,
		sel_registerName("_download:decideDestinationWithSuggestedFilename:"
			"completionHandler:"),
		(IMP)run_save_panel, "v@:@@?");
	class_addMethod(__WKDownloadDelegate,
		sel_registerName("_download:didFailWithError:"),
		(IMP)download_failed, "v@:@@");
	objc_registerClassPair(__WKDownloadDelegate);
	id downloadDelegate =
		((id(*)(id, SEL))objc_msgSend)((id)__WKDownloadDelegate, sel_registerName("new"));

	Class __WKPreferences = objc_allocateClassPair(objc_getClass("WKPreferences"),
		"__WKPreferences", 0);
	objc_property_attribute_t type = {"T", "c"};
	objc_property_attribute_t ownership = {"N", ""};
	objc_property_attribute_t attrs[] = {type, ownership};
	class_replaceProperty(__WKPreferences, "developerExtrasEnabled", attrs, 2);
	objc_registerClassPair(__WKPreferences);
	id wkPref = ((id(*)(id, SEL))objc_msgSend)((id)__WKPreferences, sel_registerName("new"));


	((void(*)(id, SEL, id, id))objc_msgSend)(wkPref, sel_registerName("setValue:forKey:"),
		((id(*)(id, SEL, int))objc_msgSend)((id)objc_getClass("NSNumber"),
			sel_registerName("numberWithBool:"), !!w->debug),
		((id(*)(id, SEL, char *))objc_msgSend)((id)objc_getClass("NSString"),
			sel_registerName("stringWithUTF8String:"),
			"developerExtrasEnabled"));


	id userController = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("WKUserContentController"),
		sel_registerName("new"));
	objc_setAssociatedObject(userController, "webview", (id)(w),
		OBJC_ASSOCIATION_ASSIGN);
	((void(*)(id, SEL, id, id))objc_msgSend)(
		userController, sel_registerName("addScriptMessageHandler:name:"),
		scriptMessageHandler,
		((id(*)(id, SEL, char *))objc_msgSend)((id)objc_getClass("NSString"),
			sel_registerName("stringWithUTF8String:"), "invoke"));

/***
 In order to maintain compatibility with the other 'webviews' we need to
 override window.external.invoke to call
 webkit.messageHandlers.invoke.postMessage
 ***/

	id windowExternalOverrideScript = ((id(*)(id, SEL))objc_msgSend)(
		(id)objc_getClass("WKUserScript"), sel_registerName("alloc"));

	((void(*)(id, SEL, id, int, int))objc_msgSend)(
		windowExternalOverrideScript,
		sel_registerName("initWithSource:injectionTime:forMainFrameOnly:"),
		(id)cocoa_str("window.external = this; invoke = function(arg){ "
			"webkit.messageHandlers.invoke.postMessage(arg); };"),
		WKUserScriptInjectionTimeAtDocumentStart, 0);

	((void(*)(id, SEL, id))objc_msgSend)(userController, sel_registerName("addUserScript:"),
		windowExternalOverrideScript);

	id config = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("WKWebViewConfiguration"),
		sel_registerName("new"));

	id processPool = ((id(*)(id, SEL))objc_msgSend)(config, sel_registerName("processPool"));
	((void(*)(id, SEL, id))objc_msgSend)(processPool, sel_registerName("_setDownloadDelegate:"),
		downloadDelegate);
	((void(*)(id, SEL, id))objc_msgSend)(config, sel_registerName("setProcessPool:"), processPool);
	((void(*)(id, SEL, id))objc_msgSend)(config, sel_registerName("setUserContentController:"),
		userController);
	((void(*)(id, SEL, id))objc_msgSend)(config, sel_registerName("setPreferences:"), wkPref);

	Class __NSWindowDelegate = objc_allocateClassPair(objc_getClass("NSObject"),
		"__NSWindowDelegate", 0);
	class_addProtocol(__NSWindowDelegate, objc_getProtocol("NSWindowDelegate"));
	class_replaceMethod(__NSWindowDelegate, sel_registerName("windowWillClose:"),
		(IMP)webview_window_will_close, "v@:@");
	objc_registerClassPair(__NSWindowDelegate);

	w->priv.windowDelegate =
		((id(*)(id, SEL))objc_msgSend)((id)__NSWindowDelegate, sel_registerName("new"));

	objc_setAssociatedObject(w->priv.windowDelegate, "webview", (id)(w),
		OBJC_ASSOCIATION_ASSIGN);

	id nsTitle =
		((id(*)(id, SEL, const char *))objc_msgSend)((id)objc_getClass("NSString"),
			sel_registerName("stringWithUTF8String:"), w->title);

	CGRect r = CGRectMake(0, 0, w->width, w->height);

	unsigned int style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
		NSWindowStyleMaskMiniaturizable;
	if (w->resizable) {
		style = style | NSWindowStyleMaskResizable;
	}

	w->priv.window =
		((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSWindow"), sel_registerName("alloc"));

	((void(*)(id, SEL, CGRect, unsigned int, int, int))objc_msgSend)(w->priv.window,
		sel_registerName("initWithContentRect:styleMask:backing:defer:"),
		r, style, NSBackingStoreBuffered, 0);

	((void(*)(id, SEL))objc_msgSend)(w->priv.window, sel_registerName("autorelease"));
	((void(*)(id, SEL, id))objc_msgSend)(w->priv.window, sel_registerName("setTitle:"), nsTitle);
	((void(*)(id, SEL, id))objc_msgSend)(w->priv.window, sel_registerName("setDelegate:"),
		w->priv.windowDelegate);
	((void(*)(id, SEL))objc_msgSend)(w->priv.window, sel_registerName("center"));

	Class __WKUIDelegate =
		objc_allocateClassPair(objc_getClass("NSObject"), "__WKUIDelegate", 0);
	class_addProtocol(__WKUIDelegate, objc_getProtocol("WKUIDelegate"));
	class_addMethod(__WKUIDelegate,
		sel_registerName("webView:runOpenPanelWithParameters:"
			"initiatedByFrame:completionHandler:"),
		(IMP)run_open_panel, "v@:@@@?");
	class_addMethod(__WKUIDelegate,
		sel_registerName("webView:runJavaScriptAlertPanelWithMessage:"
			"initiatedByFrame:completionHandler:"),
		(IMP)run_alert_panel, "v@:@@@?");
	class_addMethod(
		__WKUIDelegate,
		sel_registerName("webView:runJavaScriptConfirmPanelWithMessage:"
			"initiatedByFrame:completionHandler:"),
		(IMP)run_confirmation_panel, "v@:@@@?");
	objc_registerClassPair(__WKUIDelegate);
	id uiDel = ((id(*)(id, SEL))objc_msgSend)((id)__WKUIDelegate, sel_registerName("new"));

	Class __WKNavigationDelegate = objc_allocateClassPair(
		objc_getClass("NSObject"), "__WKNavigationDelegate", 0);
	class_addProtocol(__WKNavigationDelegate,
		objc_getProtocol("WKNavigationDelegate"));
	class_addMethod(
		__WKNavigationDelegate,
		sel_registerName(
			"webView:decidePolicyForNavigationResponse:decisionHandler:"),
		(IMP)make_nav_policy_decision, "v@:@@?");
	objc_registerClassPair(__WKNavigationDelegate);
	id navDel = ((id(*)(id, SEL))objc_msgSend)((id)__WKNavigationDelegate, sel_registerName("new"));

	w->priv.webview = ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("WKWebView"), sel_registerName("alloc"));

	((void(*)(id, SEL, CGRect, id))objc_msgSend)(w->priv.webview,
		sel_registerName("initWithFrame:configuration:"), r, config);

	((void(*)(id, SEL, id))objc_msgSend)(w->priv.webview, sel_registerName("setUIDelegate:"), uiDel);
	((void(*)(id, SEL, id))objc_msgSend)(w->priv.webview, sel_registerName("setNavigationDelegate:"), navDel);


	int comma_index;

	const char *MIMEType = parse_data_URI_content_type(w->url, &comma_index);

	if (MIMEType != NULL) {
		id NSString = (id)cocoa_str(w->url + (comma_index + 1));
		id NSData = ((id(*)(id, SEL, int))objc_msgSend)(NSString, sel_registerName("dataUsingEncoding:"), NSUTF8StringEncoding);

		((void(*)(id, SEL, id, id, id, void *))objc_msgSend)(w->priv.webview,
			sel_registerName("loadData:MIMEType:characterEncodingName:baseURL:"),
			NSData, (id)cocoa_str(MIMEType), (id)cocoa_str("UTF-8"), NULL);

		free((void *)MIMEType);
	} else {
		id nsURL = ((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSURL"),
			sel_registerName("URLWithString:"),
			(id)cocoa_str(webview_check_url(w->url)));

		((void(*)(id, SEL, id))objc_msgSend)(w->priv.webview, sel_registerName("loadRequest:"),
			((id(*)(id, SEL, id))objc_msgSend)((id)objc_getClass("NSURLRequest"),
				sel_registerName("requestWithURL:"), nsURL));
	}

	((void(*)(id, SEL, int))objc_msgSend)(w->priv.webview, sel_registerName("setAutoresizesSubviews:"), 1);
	((void(*)(id, SEL, int))objc_msgSend)(w->priv.webview, sel_registerName("setAutoresizingMask:"),
		(NSViewWidthSizable | NSViewHeightSizable));
	((void(*)(id, SEL, id))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)(w->priv.window, sel_registerName("contentView")),
		sel_registerName("addSubview:"), w->priv.webview);

	((void(*)(id, SEL))objc_msgSend)(w->priv.window, sel_registerName("orderFrontRegardless"));

	((void(*)(id, SEL, int))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
		sel_registerName("sharedApplication")),
		sel_registerName("setActivationPolicy:"),
		NSApplicationActivationPolicyRegular);

	((void(*)(id, SEL))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
		sel_registerName("sharedApplication")),
		sel_registerName("finishLaunching"));

	((void(*)(id, SEL, int))objc_msgSend)(((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSApplication"),
		sel_registerName("sharedApplication")),
		sel_registerName("activateIgnoringOtherApps:"), 1);

	w->priv.should_exit = 0;
	return 0;
}

int webview_loop(struct webview *w, int blocking) {
	id until = (blocking ? ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"),
		sel_getUid("distantFuture"))
		: ((id(*)(id, SEL))objc_msgSend)((id)objc_getClass("NSDate"),
			sel_getUid("distantPast")));

	id event = cocoa_next_event(NSApp, NSUIntegerMax, until, (id)kCFRunLoopDefaultMode, YES);
	if (event) {
		cocoa_set_with(NSApp, "sendEvent:", event);
		//cocoa_select(NSApp, "updateWindows");
	}

	return w->priv.should_exit;
}

int webview_eval(struct webview *w, const char *js) {
	((void(*)(id, SEL, id, void *))objc_msgSend)(w->priv.webview,
		sel_getUid("evaluateJavaScript:completionHandler:"),
		(id)cocoa_str(js), NULL);
	return 0;
}

void webview_set_title(struct webview *w, const char *title) {
	((void(*)(id, SEL, id))objc_msgSend)(w->priv.window, sel_getUid("setTitle:"),
		(id)cocoa_str(title));
}

FORCEINLINE void webview_set_fullscreen(struct webview *w, int fullscreen) {
	unsigned long windowStyleMask = ((unsigned long(*)(id, SEL))objc_msgSend)(
		w->priv.window, sel_getUid("styleMask"));
	int b = (((windowStyleMask & NSWindowStyleMaskFullScreen) ==
		NSWindowStyleMaskFullScreen)
		? 1
		: 0);
	if (b != fullscreen) {
		((void(*)(id, SEL, void *))objc_msgSend)(w->priv.window, sel_getUid("toggleFullScreen:"), NULL);
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
	w->priv.should_exit = 1;
}

FORCEINLINE void webview_exit(struct webview *w) {
	cocoa_set_with(NSApp, "terminate:", NSApp);
}

FORCEINLINE void webview_print_log(const char *s) { printf("%s\n", s); }
#endif