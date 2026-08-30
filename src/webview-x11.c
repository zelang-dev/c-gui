#if defined(__linux__)
#include "gui_internal.h"

struct hist {
	char *url;
	int top_row;
	struct hist *next;
};

/* General functions */
static void fwfree(struct hist *h) {
	if (h) {
		fwfree(h->next);
		free(h->url);
		free(h);
	}
}

static void adjust_scrollbars(gui_info *ui, Widget w) {
	int tr, tc, tw, th;
	Dimension ww, wh;
	float pos, size;

	XtVaGetValues(w,
		XtNtopRow, &tr,
		XtNtopCol, &tc,
		XtNtotalWidth, &tw,
		XtNtotalHeight, &th,
		XtNwidth, &ww,
		XtNheight, &wh,
		NULL);
	pos = (float)tr / (float)th;
	size = (float)wh / (float)th;
	XawScrollbarSetThumb((ui == NULL ? (Widget)get_info_app_data() : (Widget)ui->app->app_data),
		pos, size);
	pos = (float)tc / (float)tw;
	size = (float)ww / (float)tw;
}

static void new_url(Widget w, gui_info *ui, char *u) {
	struct hist *h = MwMalloc(sizeof * h);
	char *v;
	int r;
	XtVaGetValues(ui->app->wnd,
		XtNurl, &v,
		XtNtopRow, &r,
		NULL);
	if (v) v = MwStrdup(v);
	else v = MwStrdup(u);
	XtVaSetValues(ui->app->wnd,
		XtNurl, u,
		XtNtopRow, 0,
		NULL);
	XtVaGetValues(ui->app->wnd, XtNurl, &u, NULL);
	if (strcmp(u, v)) {
		h->url = v;
		h->top_row = r;
		h->next = ui->backhist;
		ui->backhist = h;
		fwfree(ui->forwhist);
		ui->forwhist = NULL;
	} else {
		MwFree(v);
		MwFree(h);
	}
	adjust_scrollbars(ui, w);
}

static void scroll_updown(Widget w, gui_info *ui, float amount) {
	Dimension height;
	int top_row, th;

	XtVaGetValues((ui == NULL ? (Widget)get_info_app_wnd() : (Widget)ui->app->wnd),
		XtNtotalHeight, &th,
		XtNheight, &height,
		XtNtopRow, &top_row,
		NULL);
	top_row += amount * height;
	if (top_row > th) top_row = th;
	if (top_row < 0) top_row = 0;
	XtVaSetValues((ui == NULL ? get_info_app_wnd() : ui->app->wnd),
		XtNtopRow, top_row,
		NULL);
	adjust_scrollbars(ui, (ui == NULL ? get_info_app_wnd() : ui->app->wnd));
}

static void web_page_down(Widget w) {
	scroll_updown(w, NULL, 0.9);
}

static void web_page_up(Widget w) {
	scroll_updown(w, NULL, -0.9);
}

static void web_scroll_down(Widget w) {
	scroll_updown(w, NULL, 0.1);
}

static void web_scroll_up(Widget w) {
	scroll_updown(w, NULL, -0.1);
}

static void quit(Widget w, XEvent *event, String *params, Cardinal *n) {
}

static void page_down(Widget w, XEvent *event, String *params, Cardinal *n) {
	web_page_down(w);
}

static void page_up(Widget w, XEvent *event, String *params, Cardinal *n) {
	web_page_up(w);
}

static void scroll_down(Widget w, XEvent *event, String *params, Cardinal *n) {
	web_scroll_down(w);
}

static void scroll_up(Widget w, XEvent *event, String *params, Cardinal *n) {
	web_scroll_up(w);
}

static void web_scroll_leftright(Widget w, float amount) {
	Dimension width;
	int top_col, pw;

	XtVaGetValues(w,
		XtNtotalWidth, &pw,
		XtNwidth, &width,
		XtNtopCol, &top_col,
		NULL);
	top_col += amount * width;
	if (top_col > pw) top_col = pw;
	if (top_col < 0) top_col = 0;
	XtVaSetValues(w,
		XtNtopCol, top_col,
		NULL);
	adjust_scrollbars(NULL, w);
}

static void scroll_left(Widget w, XEvent *event, String *params, Cardinal *n) {
	web_scroll_leftright(w, -0.1);
}

static void scroll_right(Widget w, XEvent *event, String *params, Cardinal *n) {
	web_scroll_leftright(w, 0.1);
}

static void cb_back(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	struct hist *h = ui->backhist;
	char *u;
	int r;
	char *v;
	if (h) {
		XtVaGetValues(ui->app->wnd,
			XtNurl, &v,
			XtNtopRow, &r,
			NULL);
		v = MwStrdup(v);
		XtVaSetValues(ui->app->wnd,
			XtNurl, h->url,
			XtNtopRow, h->top_row,
			NULL);
		XtVaGetValues(ui->app->wnd, XtNurl, &u, NULL);
		MwFree(h->url);
		if (strcmp(u, v)) {
			ui->backhist = h->next;
			h->url = v;
			h->top_row = r;
			h->next = ui->forwhist;
			ui->forwhist = h;
		} else {
			MwFree(h);
			MwFree(v);
		}
		adjust_scrollbars(ui, ui->app->wnd);
	}
}

static void cb_forward(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	struct hist *h = ui->forwhist;
	char *u;
	int r;
	char *v;
	if (h) {
		XtVaGetValues(ui->app->wnd,
			XtNurl, &v,
			XtNtopRow, &r,
			NULL);
		v = MwStrdup(v);
		XtVaSetValues(ui->app->wnd,
			XtNurl, h->url,
			XtNtopRow, h->top_row,
			NULL);
		XtVaGetValues(ui->app->wnd, XtNurl, &u, NULL);
		MwFree(h->url);
		if (strcmp(u, v)) {
			ui->forwhist = h->next;
			h->url = v;
			h->top_row = r;
			h->next = ui->backhist;
			ui->backhist = h;
		} else {
			MwFree(v);
			MwFree(h);
		}
		adjust_scrollbars(ui, ui->app->wnd);
	}
}

static void cb_reload(__GUI_WEBVIEW__) {
	char *u;
	XtVaGetValues(self, XtNurl, &u, NULL);
	u = MwStrdup(u);
	XtVaSetValues(self, XtNurl, u, NULL);
	MwFree(u);
	adjust_scrollbars((gui_info *)client, self);
}

static void cb_cancel(__GUI_WEBVIEW__) {
	printf("cb_cancel()\n");
}

static void cb_home(__GUI_WEBVIEW__) {
	new_url(self, (gui_info *)client, "about:webview");
}

static void cb_error(__GUI_WEBVIEW__) {
	MwErrorBox(self, "Nothing to see here.");
}

static void cb_click(__GUI_WEBVIEW__) {
	new_url(self, client, (char *)data);
}

static void cb_open(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	static char path[1024];
	char name[1024] = "";
	char *patterns[] = {"All files (*)", NULL};
	char fmt[1024] = "";
	char *extra = NULL;
	int n, ext = 0;
	char b[1024];

	if (path[0] == '\0') strcpy(path, ".");
	n = MwFileselInput(ui->topLevel, path, name, patterns, fmt, extra, ext);
	if (n) {
		MwSnprintf(b, sizeof b, "%s/%s", path, name);
		new_url(self, ui, b);
	}
}

static void cb_save(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	static char path[1024];
	char name[1024] = "";
	char *patterns[] = {"All files (*)", NULL};
	char fmt[1024] = "";
	char *extra = NULL;
	int n, ext = 0;
	char b[1024];
	char *url;

	if (path[0] == '\0') strcpy(path, ".");
	n = MwFileselInput(ui->topLevel, path, name, patterns, fmt, extra, ext);
	XtVaGetValues(ui->app->wnd, XtNurl, &url, NULL);
	if (n) {
		MwSnprintf(b, sizeof b, "%s/%s", path, name);
		MwHtmlSave(url, b);
	}
}

static void cb_url(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	char *u = (char *)data;
	if (ui->app->code)
		MwComboTextChange(ui->user_data, u);

	adjust_scrollbars(ui, self);
}

static void cb_loc(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	int i;
	char *u;

	cb_click(self, client, data);
	XtVaGetValues(ui->app->wnd, XtNurl, &u, NULL);
	for (i = 0; i < 10; i++) {
		printf("history[%d] = '%s'\n", i, (char *)ui->app->app_array[i]);
		if (!strcmp((char *)ui->app->app_array[i], u))
			break;
	}

	if (i == 10) {
		MwFree((char *)ui->app->app_array[9]);
		for (i = 9; i > 0; i--)
			ui->app->app_array[i] = ui->app->app_array[i - 1];

		ui->app->app_array[0] = MwStrdup(u);
	}

	for (i = 0; i < 10; i++) {
		printf("history[%d] = '%s'\n", i, (char *)ui->app->app_array[i]);
	}

	XtVaSetValues(self,
		XtNcomboData, ui->app->app_array,
		XtNcomboNData, 10,
		NULL);
}

static void cb_vscroll_jump(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	float top;
	int top_row, th;

	XtVaGetValues(self, XtNtotalHeight, &th, NULL);
	XtVaGetValues(self, XtNtopOfThumb, &top, NULL);
	top_row = top * th;
	XtVaSetValues(self, XtNtopRow, top_row, NULL);
}

static void cb_vscroll_scroll(__GUI_WEBVIEW__) {
	gui_info *ui = (gui_info *)client;
	int i = (long)data;
	Dimension length, height;
	int top_row, th;
	float amount;

	XtVaGetValues(self, XtNlength, &length, NULL);
	XtVaGetValues(self,
		XtNheight, &height,
		XtNtopRow, &top_row,
		XtNtotalHeight, &th,
		NULL);
	if (i < 0) {
		if ((length / -i) > 15) {
			amount = -0.1;
		} else {
			amount = -0.9;
		}
	} else {
		if ((length / i) > 15) {
			amount = 0.1;
		} else {
			amount = 0.9;
		}
	}
	scroll_updown(self, ui, amount);
}

static Widget add_command(Widget pw, _platform_cb cb, XtPointer closure, char *pm) {
	Widget w;
	Pixmap pm_return;
	Pixel color;
	XtVaGetValues(pw, XtNbackground, &color, NULL);
	w = XtVaCreateManagedWidget("tooltip",
		commandWidgetClass, pw,
		XtNshadowWidth, 0,
		XtNforeground, color,
		NULL);
	pm_return = MwLoadPixmap(XtDisplay(pw), color, pm);
	XtVaSetValues(w, XtNbitmap, pm_return, NULL);
	XtAddCallback(w, XtNcallback, cb, closure);
	return w;
}

FORCEINLINE int webview_create(gui_info *ui, webview_t *w) {
	(void)ui;
	(void)w;
	return 1;
}

FORCEINLINE int webview_loop(webview_t *w, int blocking) {
	return w->priv.should_exit;
}

FORCEINLINE void webview_set_title(webview_t *w, const char *title) {
}

FORCEINLINE void webview_set_fullscreen(webview_t *w, int fullscreen) {
}

FORCEINLINE void webview_set_color(webview_t *w, uint8_t r, uint8_t g,
	uint8_t b, uint8_t a) {
}

FORCEINLINE void webview_dialog(webview_t *w,
	enum webview_dialog_type dlgtype, int flags,
	const char *title, const char *arg,
	char *result, size_t resultsz) {
}

FORCEINLINE int webview_eval(webview_t *w, const char *js) {
	return 0;
}

FORCEINLINE void webview_dispatch(webview_t *w, webview_dispatch_fn fn,
	void *arg) {
}

FORCEINLINE void webview_exit(webview_t *w) { (void)w; }
FORCEINLINE void webview_print_log(const char *s) {
	fprintf(stderr, "%s\n", s);
}
#endif