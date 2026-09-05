/*
Copyright (C) 2002  Ulric Eriksson <ulric@siag.nu>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the Licence, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>

#ifdef NATIVE_XAW
#	include <X11/Xaw/AsciiText.h>
#	include <X11/Xaw/Command.h>
#	include <X11/Xaw/List.h>
#	include <X11/Xaw/ListP.h>
#	include <X11/Xaw/Viewport.h>
#else
#	include <Linux/Xaw95/AsciiText.h>
#	include <Linux/Xaw95/Command.h>
#	include <Linux/Xaw95/List.h>
#	include <Linux/Xaw95/ListP.h>
#	include <Linux/Xaw95/Viewport.h>
#	include <Linux/Xaw95/TraversalP.h>
#endif

#include <Linux/Mowitz.h>
#include <X11/xpm.h>
#include <Linux/Mowitz/MwCheck.h>
#include <Linux/Mowitz/MwHtmlP.h>

extern char *x_resolve_url(char *, char *);

static float floatOne = 1.0;

#define offset(field) XtOffsetOf(MwHtmlRec, html.field)
static XtResource resources[] = {
	{
		XtNurl,			/* name */
		XtCUrl,			/* class */
		XtRString,		/* type */
		sizeof(String),		/* size */
		offset(url),		/* offset */
		XtRImmediate,		/* default_type */
		(String)0		/* default_addr */
	}, {
		XtNtopRow,
		XtCTopRow,
		XtRInt,
		sizeof(int),
		offset(top_row),
		XtRImmediate,
		(XtPointer)0
	}, {
		XtNtopCol,
		XtCTopCol,
		XtRInt,
		sizeof(int),
		offset(top_col),
		XtRImmediate,
		(XtPointer)0
	}, {
		XtNtotalWidth,
		XtCTotalWidth,
		XtRInt,
		sizeof(int),
		offset(total_width),
		XtRImmediate,
		(XtPointer)0
	}, {
		XtNtotalHeight,
		XtCTotalHeight,
		XtRInt,
		sizeof(int),
		offset(total_height),
		XtRImmediate,
		(XtPointer)0
	}, {
		XtNzoom,
		XtCZoom,
		XtRFloat,
		sizeof(float),
		offset(zoom),
		XtRFloat,
		(XtPointer)&floatOne
	}, {
		XtNcallback,
		XtCCallback,
		XtRCallback,
		sizeof(XtPointer),
		offset(callbacks),
		XtRCallback,
		NULL
	}, {
		XtNchangeUrl,
		XtCChangeUrl,
		XtRCallback,
		sizeof(XtPointer),
		offset(change_url),
		XtRCallback,
		NULL
	}, {
		XtNstatus,
		XtCStatus,
		XtRWidget,
		sizeof(Widget),
		offset(status),
		XtRImmediate,
		(XtPointer)None,
	}, {
		XtNdelay,
		XtCDelay,
		XtRInt,
		sizeof(int),
		offset(delay),
		XtRImmediate,
		(XtPointer)0
	}
};
#undef offset

#define offset(field) XtOffsetOf(MwHtmlConstraintsRec, html.field)
static XtResource htmlConstraintResources[] = {
	{
		XtNhtmlObject,
		XtCHtmlObject,
		XtRPointer,
		sizeof(XtPointer),
		offset(ob),
		XtRImmediate,
		NULL
	}
};
#undef offset

/* methods */
static void DoLayout(MwHtmlWidget);
static void Resize(Widget);
static XtGeometryResult GeometryManager(Widget,
	XtWidgetGeometry *, XtWidgetGeometry *);
static void ChangeManaged(Widget);
static void Redisplay(Widget, XEvent *, Region);
static void Initialize(Widget, Widget, ArgList, Cardinal *);
static void Realize(Widget, XtValueMask *, XSetWindowAttributes *);
static void Destroy(Widget);
static Boolean ConstraintSetValues(Widget, Widget, Widget, ArgList, Cardinal *);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);

/* actions */

static void HtmlAction(Widget, XEvent *, String *, Cardinal *);
static void HtmlMotion(Widget, XEvent *, String *, Cardinal *);
static void HtmlButton(Widget, XEvent *, String *, Cardinal *);

static XtActionsRec actions[] =
{
	{"HtmlAction",		HtmlAction},
	{"HtmlMotion",		HtmlMotion},
	{"HtmlButton",		HtmlButton},
};

/* actions for text fields in forms */

static void ac_form_done(Widget, XEvent *, String *, Cardinal *);
static void ac_form_cancel(Widget, XEvent *, String *, Cardinal *);
static void ac_form_next(Widget, XEvent *, String *, Cardinal *);
static void ac_form_previous(Widget, XEvent *, String *, Cardinal *);
static void ac_form_select(Widget, XEvent *, String *, Cardinal *);

static XtActionsRec form_actions[] =
{
	{"form_done",		ac_form_done},
	{"form_cancel",		ac_form_cancel},
	{"form_next",		ac_form_next},
	{"form_previous",	ac_form_previous},
	{"form_select",		ac_form_select}
};

/* translations */
static char translations[] =
"<Key>:		HtmlAction() \n"
"<Motion>:	HtmlMotion() \n"
"<Btn1Down>:	HtmlButton() \n"
;

MwHtmlClassRec mwHtmlClassRec = {
  { /* core fields */
		/* superclass		*/	(WidgetClass)&constraintClassRec,
		/* class_name		*/	"MwHtml",
		/* widget_size		*/	sizeof(MwHtmlRec),
		/* class_initialize		*/	NULL,
		/* class_part_initialize	*/	NULL,
		/* class_inited		*/	FALSE,
		/* initialize		*/	Initialize,
		/* initialize_hook		*/	NULL,
		/* realize			*/	Realize,
		/* actions			*/	actions,
		/* num_actions		*/	XtNumber(actions),
		/* resources		*/	resources,
		/* num_resources		*/	XtNumber(resources),
		/* xrm_class		*/	NULLQUARK,
		/* compress_motion		*/	TRUE,
		/* compress_exposure	*/	TRUE,
		/* compress_enterleave	*/	TRUE,
		/* visible_interest		*/	FALSE,
		/* destroy			*/	Destroy,
		/* resize			*/	Resize,
		/* expose			*/	Redisplay,
		/* set_values		*/	SetValues,
		/* set_values_hook		*/	NULL,
		/* set_values_almost	*/	XtInheritSetValuesAlmost,
		/* get_values_hook		*/	NULL,
		/* accept_focus		*/	XawAcceptFocus,
		/* version			*/	XtVersion,
		/* callback_private		*/	NULL,
		/* tm_table			*/	translations,
		/* query_geometry		*/	XtInheritQueryGeometry,
		/* display_accelerator	*/	XtInheritDisplayAccelerator,
		/* extension		*/	NULL
	  },{
	/* composite_class fields */
		  /* geometry_manager   */    GeometryManager,
		  /* change_managed     */    ChangeManaged,
		  /* insert_child       */    XtInheritInsertChild,
		  /* delete_child       */    XtInheritDeleteChild,
		  /* extension          */    NULL
	  /* constraint_class fields */
		}, {
			/* subresources       */   htmlConstraintResources,
			/* subresource_count  */   XtNumber(htmlConstraintResources),
			/* constraint_size    */   sizeof(MwHtmlConstraintsRec),
			/* initialize         */   NULL,
			/* destroy            */   NULL,
			/* set_values         */   ConstraintSetValues,
			/* extension          */   NULL
		/* html fields */
		  }, {
			  /* empty			*/	0
			}
};

WidgetClass mwHtmlWidgetClass = (WidgetClass)&mwHtmlClassRec;

static void HtmlAction(Widget w, XEvent *event, String *params, Cardinal *n) {
	/*printf("HtmlAction(%s)\n", XtName(w))*/;
}

static void absolute_position(object_box *ob, int *x, int *y) {
	if (ob == NULL) {
		*x = *y = 0;
		return;
	}
	absolute_position(ob->parent, x, y);
	*x += ob->x;
	*y += ob->y;
}

static object_box *locate_object(object_box *ob,
	int x_off, int y_off, int x, int y) {
	object_box *ob1;
	if (ob == NULL) return NULL;

	if (ob->child == NULL &&
		y > y_off + ob->y &&
		y < y_off + ob->y + ob->h &&
		x > x_off + ob->x &&
		x < x_off + ob->x + ob->w) return ob;

	ob1 = locate_object(ob->child, x_off + ob->x, y_off + ob->y, x, y);
	if (ob1) return ob1;
	return locate_object(ob->next, x_off, y_off, x, y);
}

/* check if an object lives between ob1 and ob2 */
static int object_between(object_box *ob, object_box *ob1, object_box *ob2) {
	while (ob1) {
		if (ob1 == ob) return 1;
		if (ob1 == ob2) return 0;
		ob1 = ob1->next;
	}
	return 0;
}

static char *get_href(MwHtmlWidget hw, int x, int y) {
	int i;

	object_box *o = locate_object(hw->html.ob, 0, 0, x, y);

	if (o == NULL) {
		return NULL;
	}

	for (i = 0; i < hw->html.nref; i++) {
		if (object_between(o, hw->html.ref[i].ob1, hw->html.ref[i].ob2))
			return hw->html.ref[i].url;
	}
	return NULL;
}

/* to contains the current url which is overwritten with the new one,
   which is calculated from to and from.

   combine_url(<anything>, "http://host/") => "http://host/"
   combine_url("http://host/index.html", "demo/index.html")
		=> "http://host/demo/index.html"
*/
static void combine_url(char *to, char *from) {
	char *p;

	p = x_resolve_url(to, from);
	strcpy(to, p);
	MwFree(p);
}

static void dump_tree(object_box *ob, int level, int x_off, int y_off) {
	int i;
	if (ob == NULL) return;
	for (i = 0; i < level; i++) printf("   ");
	printf(">%d @ %d,%d< = %p", ob->type, x_off + ob->x, y_off + ob->y, ob);
	if (ob->type == MW_HTML_ROW) {
		MwHtmlRow *a = ob->data;
		printf(" row has %d children", a->nchild);
	}
	printf("\n");
	dump_tree(ob->child, level + 1, x_off + ob->x, y_off + ob->y);
	dump_tree(ob->next, level, x_off, y_off);
}

static void HtmlMotion(Widget w, XEvent *event, String *params, Cardinal *n) {
	char b[1024];
	MwHtmlWidget hw = (MwHtmlWidget)w;
	int x = event->xmotion.x + hw->html.top_col;
	int y = event->xmotion.y + hw->html.top_row;
	char *u = get_href((MwHtmlWidget)w, x, y);

	if (hw->html.url) strcpy(b, hw->html.url);
	else b[0] = '\0';
	if (u) {
		combine_url(b, u);
		XDefineCursor(XtDisplay(w), XtWindow(w), hw->html.cursor);
	} else {
		b[0] = '\0';
		XDefineCursor(XtDisplay(w), XtWindow(w), None);
	}
	if (hw->html.status) {
		MwLabelSet(hw->html.status, b);
	}
}

static void HtmlButton(Widget w, XEvent *event, String *params, Cardinal *n) {
	char b[1024];
	MwHtmlWidget hw = (MwHtmlWidget)w;
	int x = event->xbutton.x + hw->html.top_col;
	int y = event->xbutton.y + hw->html.top_row;
	char *u = get_href((MwHtmlWidget)w, x, y);
	if (u) {
		if (hw->html.url) strcpy(b, hw->html.url);
		else b[0] = '\0';
		if (u) combine_url(b, u);
		else b[0] = '\0';
		XtCallCallbacks(w, XtNcallback, (XtPointer)b);
	}
}

static void set_focus(Widget w) {
	MwHtmlWidget hw;
	object_box *ob;
	MwHtmlInput *a;
	int i;

	if (w == None) return;
	hw = (MwHtmlWidget)XtParent(w);
	if (hw == None) return;
	for (i = 0; i < hw->composite.num_children; i++) {
		XtVaGetValues(hw->composite.children[i],
			XtNhtmlObject, &ob, (char *)0);
		if (ob && ob->type == MW_HTML_INPUT) {
			a = ob->data;
			if (a && (a->type == MW_HTML_INPUT_TEXT ||
				a->type == MW_HTML_INPUT_PASSWORD)) {
				XtVaSetValues(hw->composite.children[i],
					XtNdisplayCaret, False, (char *)0);
			}
		}
	}
	XtSetKeyboardFocus((Widget)hw, w);
	XtVaSetValues(w, XtNdisplayCaret, True, (char *)0);
}

static void cb_input_submit(Widget, XtPointer, XtPointer);
static void cb_input_reset(Widget, XtPointer, XtPointer);

static void ac_form_done(Widget w, XEvent *event, String *params, Cardinal *n) {
	cb_input_submit(w, (XtPointer)XtParent(w), NULL);;
}

static void ac_form_cancel(Widget w, XEvent *event, String *params, Cardinal *n) {
	cb_input_reset(w, (XtPointer)XtParent(w), NULL);
}

static void ac_form_next(Widget w, XEvent *event, String *params, Cardinal *n) {
	MwHtmlWidget hw = (MwHtmlWidget)XtParent(w);
	int i, j;
	object_box *ob;
	MwHtmlInput *a;

	for (i = 0; i < hw->composite.num_children; i++) {
		if (hw->composite.children[i] == w) break;
	}
	if (i == hw->composite.num_children) return;
	if (i == hw->composite.num_children - 1) j = 0;
	else j = i + 1;
	while (j != i) {
		ob = NULL;
		XtVaGetValues(hw->composite.children[j],
			XtNhtmlObject, &ob, (char *)0);
		if (ob && ob->type == MW_HTML_INPUT) {
			a = ob->data;
			if (a && (a->type == MW_HTML_INPUT_TEXT ||
				a->type == MW_HTML_INPUT_PASSWORD)) {
				set_focus(hw->composite.children[j]);
				return;
			}
		}
		if (j == hw->composite.num_children - 1) j = 0;
		else j++;
	}
}

static void ac_form_previous(Widget w, XEvent *event, String *params, Cardinal *n) {
	MwHtmlWidget hw = (MwHtmlWidget)XtParent(w);
	int i, j;
	object_box *ob;
	MwHtmlInput *a;

	for (i = 0; i < hw->composite.num_children; i++) {
		if (hw->composite.children[i] == w) break;
	}
	if (i == hw->composite.num_children) return;
	if (i == 0) j = hw->composite.num_children - 1;
	else j = i - 1;
	while (j != i) {
		ob = NULL;
		XtVaGetValues(hw->composite.children[j],
			XtNhtmlObject, &ob, (char *)0);
		if (ob && ob->type == MW_HTML_INPUT) {
			a = ob->data;
			if (a && (a->type == MW_HTML_INPUT_TEXT ||
				a->type == MW_HTML_INPUT_PASSWORD)) {
				set_focus(hw->composite.children[j]);
				return;
			}
		}
		if (j == 0) j = hw->composite.num_children - 1;
		else j--;
	}
}

static void ac_form_select(Widget w, XEvent *event, String *params, Cardinal *n) {
	set_focus(w);
}

static GC get_gc(Widget w, unsigned long fg, unsigned long bg, Font font) {
	unsigned long valuemask = 0;
	XGCValues values;
	GC gc = XCreateGC(XtDisplay(w), XtWindow(w), valuemask, &values);

	XSetForeground(XtDisplay(w), gc, fg);
	XSetBackground(XtDisplay(w), gc, bg);
	if (font != -1)
		XSetFont(XtDisplay(w), gc, font);
	return gc;
}

#define superclass (&coreClassRec)
static void Realize(Widget w, XtValueMask *valueMask,
	XSetWindowAttributes *attributes) {
	MwHtmlWidget rtw = (MwHtmlWidget)w;
	unsigned long fg, bg, blockbg;
	XColor screen_color, exact_color;
	Display *dpy = XtDisplay(w);
	char *name, *class;
	XIMStyles *styles;
	int i;

	(*superclass->core_class.realize)(w, valueMask, attributes);
	fg = BlackPixelOfScreen(XtScreen(w));
	bg = rtw->core.background_pixel;
	XAllocNamedColor(dpy,
		DefaultColormap(dpy, DefaultScreen(dpy)),
		"grey", &screen_color, &exact_color);
	blockbg = screen_color.pixel;
	rtw->html.clear_gc = get_gc(w, bg, fg, -1);
	rtw->html.cell_gc = get_gc(w, fg, blockbg,
		-1/*get_font(0)*/);

#ifdef HAVE_XCREATEIC
	/* Set up input methods */
	XtGetApplicationNameAndClass(dpy, &name, &class);
	rtw->html.xim = XOpenIM(dpy, XtDatabase(dpy), name, class);
	XGetIMValues(rtw->html.xim,
		XNQueryInputStyle, &styles,
		(char *)0);
	for (i = 0; i < styles->count_styles; i++) {
		if (styles->supported_styles[i] ==
			(XIMPreeditNothing | XIMStatusNothing)) break;
	}
	if (i == styles->count_styles) i = 0;
	rtw->html.xic = XCreateIC(rtw->html.xim,
		XNInputStyle, styles->supported_styles[i],
		XNClientWindow, XtWindow(w),
		(char *)0);
#endif
}

static void Initialize(Widget request, Widget new, ArgList args, Cardinal *n) {
	MwHtmlWidget rtw = (MwHtmlWidget)new;
	int init_done = 0;

	if (!init_done) {
		XtAppContext ac = XtWidgetToApplicationContext(new);
		XtAppAddActions(ac, form_actions, XtNumber(form_actions));
		init_done = 1;
	}

	rtw->html.timeout = None;
	rtw->html.ref = NULL;
	rtw->html.nref = 0;
	rtw->html.ob = NULL;
	if (rtw->core.width == 0) rtw->core.width = 200;
	if (rtw->core.height == 0) rtw->core.height = 200;
	rtw->html.cursor = XCreateFontCursor(XtDisplay(new), XC_hand2);
	img_read(DEFAULT_DATAPATH"/missing.png");
	rtw->html.fish = img_pop();
	if (rtw->html.fish == NULL)
		printf("missing failed\n");
	else printf("missing successful, size = %dx%d\n",
		rtw->html.fish->width, rtw->html.fish->height);
}

static void free_tree(MwHtmlNode *t) {
	if (t) {
		free_tree(t->child);
		free_tree(t->next);
		MwFree(t);
	}
}

static void free_text(MwHtmlWidget hw) {
	int i;
	/* free the hyperlinks as well, because they will be rebuilt */
	for (i = 0; i < hw->html.nref; i++) {
		MwFree(hw->html.ref[i].url);
	}
	MwFree(hw->html.ref);
	hw->html.ref = NULL;
	hw->html.nref = 0;

	for (i = 0; i < hw->composite.num_children; i++) {
		XtDestroyWidget(hw->composite.children[i]);
	}
	MwHtmlFree(hw->html.ob);
	hw->html.ob = NULL;
}

static void Destroy(Widget w) {
	MwHtmlWidget rtw = (MwHtmlWidget)w;

	XFreeGC(XtDisplay(w), rtw->html.clear_gc);
	XFreeGC(XtDisplay(w), rtw->html.cell_gc);
	img_free(rtw->html.fish);
#ifdef HAVE_XCREATEIC
	XDestroyIC(rtw->html.xic);
	XCloseIM(rtw->html.xim);
#endif
	if (rtw->html.timeout)
		XtRemoveTimeOut(rtw->html.timeout);
	free_text(rtw);
}

struct hs_out {
	float x, y/*, xmax*/;	/* where to put the next char */
	int il;			/* indentation level */
	object_box *fob;	/* first obj in current line */
	MwHtmlWidget w;
	GC g;
	float z;		/* zoom; currently unused (always 1.0 */
	href ref;
	int refstate;		/* 0 = outside, 1 = inside ref */
};

static void begin_a(void *closure, object_box *ob, MwHtmlAnchor *a) {
	struct hs_out *o = closure;
	MwHtmlWidget hw = o->w;
	int nref = hw->html.nref;
	o->refstate = 0;
	if (a->href == NULL && a->name == NULL) return;
	hw->html.ref = MwRealloc(hw->html.ref, (nref + 1) * sizeof(href));
	if (a->href) o->ref.url = MwStrdup(a->href);
	else o->ref.url = NULL;
	if (a->name) o->ref.name = MwStrdup(a->name);
	else o->ref.name = NULL;
	o->ref.ob1 = ob;
	o->refstate = 1;
}

static void end_a(void *closure, object_box *ob) {
	struct hs_out *o = closure;
	MwHtmlWidget hw = o->w;
	int nref = hw->html.nref;
	if (o->refstate == 0) {
		fprintf(stderr, "Error: </a> not in ref\n");
		return;
	}
	hw->html.nref = nref + 1;
	o->ref.ob2 = ob;
	hw->html.ref[nref] = o->ref;
}

static object_box *next_up(object_box *ob) {
	if (ob == NULL) return NULL;
	if (ob->next) return ob->next;
	return next_up(ob->parent);
}

static float breakline(object_box *fob, /*struct hs_out *o,*/ object_box *ob) {
	float w, mh = 0;
	object_box *ob1;

	/* eliminate spaces at the beginning of the line */
	ob1 = /*o->*/fob;
	w = 0;
	while (ob1 && ob1 != ob &&
		(ob1->w == 0 || ob1->type == MW_HTML_SPACE)) {
		w += ob1->w;
		ob1->w = 0;
		ob1 = ob1->next;
	}
	if (ob1 == NULL) return 0;
	while (ob1 != ob) {
		ob1->x -= w;
		ob1 = ob1->next;
	}

	for (ob1 = /*o->*/fob; ob1 != ob; ob1 = ob1->next) {
		if (ob1->type == MW_HTML_SPACE) {
			ob1->w = 0;
		}
	}

	for (ob1 = /*o->*/fob; ob1 != ob; ob1 = ob1->next) {
		if (ob1->h > mh) {
			mh = ob1->h;
		}
	}

	for (ob1 = /*o->*/fob; ob1 != ob; ob1 = ob1->next) {
		ob1->y = ob1->y + mh - ob1->h;
	}

	return mh;
}

static void draw_pic(MwHtmlWidget hw, Drawable d, image *img, int x, int y) {
	int i, j;
	XImage *im_out;
	GC gc = hw->html.cell_gc;
	Display *dpy = XtDisplay((Widget)hw);
	int screen = DefaultScreen(dpy);
	Visual *visual = DefaultVisual(dpy, screen);
	int bitmap_pad = XBitmapPad(dpy);
	int format = ZPixmap;
	int offset = 0;
	char *data = NULL;
	int width, height, depth;
	int bytes_per_line = 0;
	XColor color;
	pixel p;

	if (img == NULL) return;
	width = img->width;
	height = img->height;
	depth = hw->core.depth;
	if (img->_image) {
		im_out = XCreateImage(dpy, visual, depth, format, offset, img->_image,
			width, height, bitmap_pad, bytes_per_line);
		XPutImage(dpy, d, gc, im_out, 0, 0, x, y, img->width, img->height);
		im_out->data = NULL;
	} else {
		im_out = XCreateImage(dpy, visual, depth, format, offset, data,
			width, height, bitmap_pad, bytes_per_line);
		im_out->data = MwMalloc(im_out->bytes_per_line * im_out->height);
		for (i = 0; i < height; i++) {
			for (j = 0; j < width; j++) {
				p = GET_PIXEL(img, j, i);
				color.red = 257 * p.r;
				color.green = 257 * p.g;
				color.blue = 257 * p.b;
				MwAllocColor(dpy, None, &color);
				XPutPixel(im_out, j, i, color.pixel);
			}
		}
		XPutImage(dpy, d, gc, im_out, 0, 0,
			x, y, img->width, img->height);
	}
	XDestroyImage(im_out);
}

static image *load_pic(char *base, char *url) {
	char *nurl;
	image *img;

	nurl = x_resolve_url(base, url);
	img = img_load(nurl);
	MwFree(nurl);
	return img;
}

static void reset_input(Widget w, object_box *ob) {
	MwHtmlInput *a;
	if (w == None || ob == NULL || ob->type != MW_HTML_INPUT) return;
	a = ob->data;
	switch (a->type) {
		case MW_HTML_INPUT_TEXT:
		case MW_HTML_INPUT_PASSWORD:
			MwTextFieldSetString(w, a->value);
			break;
		case MW_HTML_INPUT_TEXTAREA:
			XtVaSetValues(w, XtNstring, a->value, (char *)0);
			break;
	}
}

static char *quote_input(char *p) {
	char *q;
	int i, j;

	q = strchr(p, '=');
	if (!q) return MwStrdup(p);
	q++;	/* past = */
	j = q - p;
	i = j;
	for (; *q; q++) {
		if (isalnum(*q)) i++;
		else i += 3;
	}
	q = MwMalloc(i + 2);
	memcpy(q, p, j);
	i = j;
	while (p[i]) {
		if (isalnum(p[i])) q[j++] = p[i++];
		else if (p[i] == ' ') q[j++] = '+', i++;
		else sprintf(q + j, "%%%02x", (int)p[i++]), j += 3;
	}
	q[j] = '\0';
	return q;
}

static char *get_input(Widget w, object_box *ob) {
	Boolean state;
	MwHtmlInput *a;
	char *value = NULL, *result;
	if (w == None || ob == NULL || ob->type != MW_HTML_INPUT) return NULL;
	a = ob->data;
	switch (a->type) {
		case MW_HTML_INPUT_TEXT:
		case MW_HTML_INPUT_PASSWORD:
			value = MwTextFieldGetString(w);
			value = MwStrdup(value);
			break;
		case MW_HTML_INPUT_TEXTAREA:
			value = NULL;
			XtVaGetValues(w, XtNstring, &value, (char *)0);
			if (value) value = MwStrdup(value);
			break;
		case MW_HTML_INPUT_RADIO:
		case MW_HTML_INPUT_CHECKBOX:
			state = False;
			XtVaGetValues(w, XtNstate, &state, (char *)0);
			if (state) value = MwStrdup(a->value);
			break;
		case MW_HTML_INPUT_HIDDEN:
			value = MwStrdup(a->value);
			break;
		case MW_HTML_INPUT_SELECT:
			value = NULL;
			XtVaGetValues(w, XtNlabel, &value, (char *)0);
			if (value) value = MwStrdup(value);
			break;
		case MW_HTML_INPUT_SELECT2:
			printf("FIXME: we don't handle multiple selection lists yet\n");
			break;
	}
	if (value == NULL) return NULL;
	result = MwMalloc(strlen(a->name) + strlen(value) + 2);
	sprintf(result, "%s=%s", a->name, value);
	MwFree(value);
	return result;
}

static void cb_input_submit(Widget w,
	XtPointer client_data, XtPointer call_data) {
	int i;
	char *action, *p, *q;
	int sep = '?';
	MwHtmlInput *a, *a1;
	object_box *ob = NULL, *ob1 = NULL;
	MwHtmlWidget hw = (MwHtmlWidget)client_data;
	XtVaGetValues(w, XtNhtmlObject, &ob, (char *)0);
	if (ob == NULL || ob->type != MW_HTML_INPUT) return;
	a = ob->data;
	if (a == NULL) return;
	action = MwStrdup(a->action);
	for (i = 0; i < hw->composite.num_children; i++) {
		Widget child = hw->composite.children[i];
		ob1 = NULL;
		XtVaGetValues(child, XtNhtmlObject, &ob1, (char *)0);
		if (ob1 == NULL) continue;
		a1 = ob1->data;
		if (a1 && a->action == a1->action) {
			q = get_input(child, ob1);
			if (q) {
				char *t = quote_input(q);
				MwFree(q);
				p = MwMalloc(strlen(action) + strlen(t) + 3);
				sprintf(p, "%s%c%s", action, sep, t);
				MwFree(t);
				MwFree(action);
				action = p;
				sep = '&';
			}
		}
	}
	XtCallCallbacks((Widget)hw, XtNcallback, (XtPointer)action);
	MwFree(action);
}

static void cb_input_reset(Widget w,
	XtPointer client_data, XtPointer call_data) {
	MwHtmlWidget hw = (MwHtmlWidget)client_data;
	object_box *ob = NULL, *ob1 = NULL;
	MwHtmlInput *a, *a1;
	int i;
	XtVaGetValues(w, XtNhtmlObject, &ob, (char *)0);
	if (ob == NULL || ob->type != MW_HTML_INPUT) return;
	a = ob->data;
	if (a == NULL) return;
	for (i = 0; i < hw->composite.num_children; i++) {
		Widget child = hw->composite.children[i];
		ob1 = NULL;
		XtVaGetValues(child, XtNhtmlObject, &ob1, (char *)0);
		if (ob1 == NULL) continue;
		a1 = ob1->data;
		if (a1 && a->action == a1->action) {
			reset_input(child, ob1);
		}
	}
}

static void cb_input_radio(Widget w,
	XtPointer client_data, XtPointer call_data) {
	object_box *ob = NULL, *ob1 = NULL;
	MwHtmlWidget hw = (MwHtmlWidget)client_data;
	MwHtmlInput *a, *a1;
	int i;

	XtVaGetValues(w, XtNhtmlObject, &ob, (char *)0);
	if (ob == NULL) return;		/* shouldn't happen */
	a = ob->data;
	if (a == NULL) return;		/* shouldn't happen */
	for (i = 0; i < hw->composite.num_children; i++) {
		Widget child = hw->composite.children[i];
		ob1 = NULL;
		XtVaGetValues(child, XtNhtmlObject, &ob1, (char *)0);
		if (ob1 == NULL) continue;	/* weird, but ok... */
		a1 = ob1->data;
		if (a1 && a1->type == MW_HTML_INPUT_RADIO &&
			!strcmp(a->name, a1->name) && a->action == a1->action) {
			XtVaSetValues(child, XtNstate, False, (char *)0);
		}
	}
	XtVaSetValues(w, XtNstate, True, (char *)0);
}

static void cb_input_select(Widget w,
	XtPointer client_data, XtPointer call_data) {
	Widget button = (Widget)client_data;
	char *label;
	label = NULL;
	XtVaGetValues(w, XtNlabel, &label, (char *)0);
	if (label == NULL) return;
	XtVaSetValues(button, XtNlabel, label, (char *)0);
}

static void create_input(Widget parent, object_box *ob, MwHtmlInput *a) {
	char **list;
	int nlist;
	Widget w, button, menu;
	w = None;
	switch (a->type) {
		case MW_HTML_INPUT_TEXT:
			ob->w = ob->min_w = ob->max_w = 200;
			ob->h = 22;
			w = XtVaCreateManagedWidget("Text",
				mwTextfieldWidgetClass, parent,
				XtNhtmlObject, ob,
				XtNborderWidth, 1,
				XtNdisplayCaret, False,
				(char *)0);
			break;
		case MW_HTML_INPUT_PASSWORD:
			ob->w = ob->min_w = ob->max_w = 200;
			ob->h = 22;
			w = XtVaCreateManagedWidget("Text",
				mwTextfieldWidgetClass, parent,
				XtNhtmlObject, ob,
				XtNborderWidth, 1,
				XtNdisplayCaret, False,
				XtNecho, False,
				(char *)0);
			break;
		case MW_HTML_INPUT_TEXTAREA:
			ob->w = ob->min_w = ob->max_w = 400;
			ob->h = 100;
			w = XtVaCreateManagedWidget("Textarea",
				asciiTextWidgetClass, parent,
				XtNhtmlObject, ob,
				XtNeditType, XawtextEdit,
				XtNscrollHorizontal, XawtextScrollWhenNeeded,
				(char *)0);
			break;
		case MW_HTML_INPUT_CHECKBOX:
			ob->w = ob->min_w = ob->max_w = 20;
			ob->h = 20;
			w = XtVaCreateManagedWidget("Check",
				mwCheckWidgetClass, parent,
				XtNcheckStyle, MwCheckWin,
				XtNhtmlObject, ob,
				XtNborderWidth, 0,
				(char *)0);
			break;
		case MW_HTML_INPUT_RADIO:
			ob->w = ob->min_w = ob->max_w = 20;
			ob->h = 20;
			w = XtVaCreateManagedWidget("Radio",
				mwCheckWidgetClass, parent,
				XtNcheckStyle, MwRadioWin,
				XtNhtmlObject, ob,
				XtNborderWidth, 0,
				(char *)0);
			XtAddCallback(w, XtNcallback, cb_input_radio, parent);
			break;
		case MW_HTML_INPUT_SELECT:
			ob->w = ob->min_w = ob->max_w = 100;
			ob->h = 20;
			list = (char **)a->value;
			button = XtVaCreateManagedWidget("form_select",
				mwMenuButtonWidgetClass, parent,
				XtNmenu_name, "form_menu",
				XtNlabel, list[0],
				XtNhtmlObject, ob,
				(char *)0);
			menu = XtVaCreatePopupShell("form_menu",
				mwMenuWidgetClass, parent,
				(char *)0);
			for (nlist = 0; list[nlist]; nlist++) {
				w = XtVaCreateManagedWidget(list[nlist],
					mwLabelMEObjectClass, menu,
					XtNlabel, list[nlist],
					(char *)0);
				XtAddCallback(w, XtNcallback, cb_input_select,
					(XtPointer)button);
			}
			break;
		case MW_HTML_INPUT_SELECT2:
			ob->w = ob->min_w = ob->max_w = 100;
			ob->h = 100;
			w = XtVaCreateManagedWidget("form_viewport",
				viewportWidgetClass, parent,
				XtNallowHoriz, True,
				XtNallowVert, True,
				XtNuseBottom, True,
				XtNuseRight, True,
				XtNforceBars, True,
				XtNhtmlObject, ob,
				XtNwidth, 100, XtNheight, 100,
				(char *)0);
			w = XtVaCreateManagedWidget("form_list",
				listWidgetClass, w,
				XtNdefaultColumns, 1,
				XtNforceColumns, 1,
				(char *)0);
			list = (char **)a->value;
			for (nlist = 0; list[nlist]; nlist++);
			XawListChange(w, list, nlist, 0, True);
			break;
		case MW_HTML_INPUT_SUBMIT:
			ob->w = ob->min_w = ob->max_w = 80;
			ob->h = 20;
			w = XtVaCreateManagedWidget("Submit",
				commandWidgetClass, parent,
				XtNhtmlObject, ob,
				XtNborderWidth, 0,
				XtNlabel, a->name ? a->name : "Submit",
				(char *)0);
			XtAddCallback(w, XtNcallback, cb_input_submit, parent);
			break;
		case MW_HTML_INPUT_IMAGE:
			printf("Can't create image input\n");
			break;
		case MW_HTML_INPUT_RESET:
			ob->w = ob->min_w = ob->max_w = 80;
			ob->h = 20;
			w = XtVaCreateManagedWidget("Reset",
				commandWidgetClass, parent,
				XtNhtmlObject, ob,
				XtNborderWidth, 0,
				(char *)0);
			XtAddCallback(w, XtNcallback, cb_input_reset, parent);
			break;
		case MW_HTML_INPUT_BUTTON:
			printf("Can't create button input\n");
			break;
		case MW_HTML_INPUT_HIDDEN:
			w = XtVaCreateWidget("Hidden",
				coreWidgetClass, parent,
				XtNhtmlObject, ob,
				(char *)0);
			break;
		case MW_HTML_INPUT_FILE:
			printf("Can't create file input\n");
			break;
		default:
			printf("Unknown input: %d\n", a->type);
	}
	reset_input(w, ob);
}

static void size_objects(MwHtmlWidget rtw, struct hs_out *o, object_box *ob) {
	int table_level, table_rows, table_cols, table_max_cols;
	object_box *table_start, *table_end, *ob1, *ob2;
	image *img;
	MwRichchar *rc;
	MwRichchar space[2] = {{' ', 0}, {'\0', 0}};
	MwHtmlRow *row_a;
	MwHtmlTable *table_a;
	int col;

	table_level = table_rows = table_cols = table_max_cols = 0;
	table_start = table_end = NULL;

	if (ob == NULL) {
		return;	/* nothing to do */
	}

	/* calculate size of all objects */

	for (/*ob = rtw->html.ob*/; ob; ob = ob->next) {
		ob->h = 0;
		ob->min_w = ob->max_w = 0;
		switch (ob->type) {
			case MW_HTML_BEGIN_A:
				begin_a(o, ob, ob->data);
				break;
			case MW_HTML_END_A:
				end_a(o, ob);
				break;
			case MW_HTML_IMAGE:
				img = load_pic(rtw->html.url,
					((MwHtmlImage *)ob->data)->src);
				if (img) {
					ob->min_w = ob->max_w = img->width;
					ob->h = img->height;
					img_free(img);
				} else if (rtw->html.fish) {
					ob->min_w = ob->max_w = rtw->html.fish->width;
					ob->h = rtw->html.fish->height;
				}

				break;
			case MW_HTML_INPUT:
				create_input((Widget)rtw, ob, ob->data);
				break;
			case MW_HTML_INDENT:
				o->il = (intptr_t)ob->data;
				ob->min_w = ob->max_w = 0;
				break;
			case MW_HTML_HR:
				ob->min_w = ob->max_w = rtw->core.width;
				ob->h = 1;
				break;
			case MW_HTML_WORDPART:
				rc = ob->data;
				ob->h = MwRcStrheight(rc, -1);
				ob->min_w = ob->max_w = MwRcStrwidth(rc, -1);
				break;
			case MW_HTML_SPACE:
				rc = ob->data;
				space[0].fmt = rc[0].fmt;
				ob->h = MwRcStrheight(space, -1);
				ob->min_w = ob->max_w = MwRcStrwidth(space, -1);
				break;
			case MW_HTML_CELL:
				size_objects(rtw, o, ob->child);
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					if (ob1->min_w > ob->min_w)
						ob->min_w = ob1->min_w;
					ob->max_w += ob1->max_w;
				}
				break;
			case MW_HTML_ROW:
				row_a = ob->data = MwMalloc(sizeof * row_a);
				row_a->nchild = 0;
				size_objects(rtw, o, ob->child);
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					row_a->nchild++;
				}
				break;
			case MW_HTML_TABLE:
				table_a = ob->data = MwMalloc(sizeof * table_a);
				size_objects(rtw, o, ob->child);
				table_a->max_child = 0;
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					row_a = ob1->data;
					if (row_a->nchild > table_a->max_child) {
						table_a->max_child = row_a->nchild;
					}
				}

				table_a->colwidth = MwMalloc(table_a->max_child *
					sizeof * (table_a->colwidth));
				for (col = 0; col < table_a->max_child; col++) {
					table_a->colwidth[col].min = 0;
					table_a->colwidth[col].max = 0;
				}
				/* get largest min_w/max_w from each column */
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					col = 0;
					for (ob2 = ob1->child; ob2; ob2 = ob2->next) {
						if (ob2->min_w > table_a->colwidth[col].min)
							table_a->colwidth[col].min = ob2->min_w;
						if (ob2->max_w > table_a->colwidth[col].max)
							table_a->colwidth[col].max = ob2->max_w;
						col++;
					}
				}
				for (col = 0; col < table_a->max_child; col++) {
					ob->min_w += table_a->colwidth[col].min;
					ob->max_w += table_a->colwidth[col].max;
				}
				break;
			default:
				/* workaround for the new parser */
				if (ob->child) {
					printf("you are not allowed to have children\n");
					exit(0);
					size_objects(rtw, o, ob->child);
				}
				break;
		}
	}
}

static void assign_sizes(MwHtmlWidget rtw, /*struct hs_out *o,*/ object_box *ob) {
	MwHtmlTable *table_a; int col;
	object_box *fob, *ob1, *ob2;
	int x, y;
	int W, D, d;

	if (ob == NULL) {
		return;
	}

	for (; ob; ob = ob->next) {
		switch (ob->type) {
			case MW_HTML_TABLE:
				table_a = ob->data;
				ob->w = 0;
				if (ob->min_w > ob->parent->w) {
					for (col = 0; col < table_a->max_child; col++) {
						table_a->colwidth[col].real =
							table_a->colwidth[col].min;
					}
				} else if (ob->max_w < ob->parent->w) {
					for (col = 0; col < table_a->max_child; col++) {
						table_a->colwidth[col].real =
							table_a->colwidth[col].max;
					}
				} else {
					W = ob->parent->w - ob->min_w;
					D = ob->max_w - ob->min_w;
					if (D == 0) D = 1;	/* no div by zero */
					for (col = 0; col < table_a->max_child; col++) {
						d = table_a->colwidth[col].max -
							table_a->colwidth[col].min;
						table_a->colwidth[col].real =
							table_a->colwidth[col].min + d * W / D;
					}
				}
				for (col = 0; col < table_a->max_child; col++) {
					ob->w += table_a->colwidth[col].real;
				}
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					ob1->w = ob->w; ob1->x = 0;	/* always */
					col = 0;
					x = 0;
					for (ob2 = ob1->child; ob2; ob2 = ob2->next) {
						ob2->w = table_a->colwidth[col].real;
						ob2->x = x;
						x += ob2->w;
						ob2->y = 0;     /* always */
						col++;
					}
				}
				assign_sizes(rtw, /*o,*/ ob->child);
				ob->h = 0;
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					ob1->y = ob->h;
					ob->h += ob1->h;
				}
				break;
			case MW_HTML_ROW:
				/* widths were done for us by the table, so we
				   only need to calculate our height */
				assign_sizes(rtw, /*o,*/ ob->child); /* do cells first */
				ob->h = 0;
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					if (ob1->h > ob->h) ob->h = ob1->h;
				}
				break;
			case MW_HTML_CELL:
				/* widths were done by the table, so we
				   only need to calculate our height */
				assign_sizes(rtw, /*o,*/ ob->child); /* can be tables! */
				x = y = 0;
				fob = ob->child;	/* first in line */
				for (ob1 = ob->child; ob1; ob1 = ob1->next) {
					if (ob1->type == MW_HTML_NEWLINE ||
						x + ob1->w > ob->w) {
						y += breakline(fob, ob1);
						x = 0;	/* FIXME: should be indent */
						fob = ob1;
					}
					ob1->x = x;
					x += ob1->w;
					ob1->y = y;
				}
				if (x) y += breakline(fob, NULL);
				ob->h = y;
				break;
			default:
				ob->w = ob->min_w;
		}
	}
}

static void draw_objects(MwHtmlWidget rtw, Drawable d, int x_off, int y_off,
	/*struct hs_out *o,*/ object_box *ob) {
	image *img;
	MwRichchar *rc;
	MwRichchar space[2] = {{' ', 0}, {'\0', 0}};
	char b[100];

	/* draw all relevant objects */
	for (; ob; ob = ob->next) {
		if (ob->y + ob->h + y_off < 0) continue;
		if (ob->y + y_off > rtw->core.height) continue;
		switch (ob->type) {
			case MW_HTML_IMAGE:
				img = load_pic(rtw->html.url,
					((MwHtmlImage *)ob->data)->src);
				if (img) {
					draw_pic(rtw, d, img,
						ob->x + x_off, ob->y + y_off);
					img_free(img);
				} else {
					draw_pic(rtw, d, rtw->html.fish,
						ob->x + x_off, ob->y + y_off);
				}

				break;
			case MW_HTML_HR:
				break;
			case MW_HTML_WORDPART:
				rc = ob->data;
				MwRcStrdraw(d, rtw->html.cell_gc, ob->x, ob->y + ob->h - 2,
					x_off, y_off, rc, -1, rtw->html.zoom);
				break;
			case MW_HTML_SPACE:
				rc = ob->data;
				space[0].fmt = rc[0].fmt;
				MwRcStrdraw(d, rtw->html.cell_gc, ob->x, ob->y + ob->h - 2,
					x_off, y_off, space, -1, rtw->html.zoom);
				break;
			case MW_HTML_UBULLET:
				rc = MwRcMakerich("*", 0);
				MwRcStrdraw(d, rtw->html.cell_gc, ob->x - 20, ob->y + ob->h - 2,
					x_off, y_off, rc, -1, rtw->html.zoom);
				MwFree(rc);
				break;
			case MW_HTML_OBULLET:
				sprintf(b, "%d.", (intptr_t)ob->data);
				rc = MwRcMakerich(b, 0);
				MwRcStrdraw(d, rtw->html.cell_gc, ob->x - 20, ob->y + ob->h - 2,
					x_off, y_off, rc, -1, rtw->html.zoom);
				MwFree(rc);
				break;
			default:
				/* workaround for the new parser */
				if (ob->child) {
					draw_objects(rtw, d,
						x_off + ob->x, y_off + ob->y,
						ob->child);
				}
		}
	}
}

static void MwHtmlDraw(MwHtmlWidget rtw, Drawable d) {
	struct hs_out o;

	if (rtw->html.ob == NULL) {
		rtw->html.ob = MwHtmlParse(rtw->html.url);
		if (rtw->html.ob == NULL)
			rtw->html.ob = MwHtmlParse("about:error");
		if (rtw->html.ob == NULL) return;

		o.il = 0;
		o.x = 0;
		o.y = 0;
		o.w = rtw;
		o.g = rtw->html.cell_gc;
		o.z = rtw->html.zoom;
		o.fob = rtw->html.ob;
		rtw->html.ob->w = rtw->core.width;
		rtw->html.ob->h = 0;	/* we don't care yet */
		rtw->html.ob->x = rtw->html.ob->y = 0;
		size_objects(rtw, &o, rtw->html.ob);
		assign_sizes(rtw, /*&o,*/ rtw->html.ob);
		rtw->html.total_width = rtw->html.ob->w;
		rtw->html.total_height = rtw->html.ob->h;

		XtCallCallbacks((Widget)rtw, XtNchangeUrl,
			(XtPointer)rtw->html.url);
	}

#if 0
	dump_tree(rtw->html.ob, 1, 0, 0);
#endif
	draw_objects(rtw, d, -rtw->html.top_col, -rtw->html.top_row,
		/*&o,*/ rtw->html.ob);
}

/* ---
Draw onto a pixmap.
*/

static Pixmap html_pixmap(MwHtmlWidget rtw) {
	int width, height, depth;
	Pixmap scribble;
	Widget w = (Widget)rtw;

	width = rtw->core.width;
	height = rtw->core.height;
	depth = rtw->core.depth;
	if (width > 2000 || height > 2000) return None;

	scribble = XCreatePixmap(XtDisplay(w), XtWindow(w),
		width, height, depth);
	XFillRectangle(XtDisplay(w), scribble, rtw->html.clear_gc,
		0, 0, width, height);
	MwHtmlDraw(rtw, scribble);
	return scribble;
}

static void do_redisplay(XtPointer client_data, XtIntervalId *id) {
	Widget w = (Widget)client_data;
	Pixmap scribble;
	GC gc;
	unsigned long valuemask = 0;
	XGCValues values;
	MwHtmlWidget rtw = (MwHtmlWidget)w;

	scribble = html_pixmap(rtw);
	if (scribble == None) return;
	gc = XCreateGC(XtDisplay(w), XtWindow(w),
		valuemask, &values);
	XCopyArea(XtDisplay(w), scribble, XtWindow(w),
		gc, 0, 0, rtw->core.width, rtw->core.height, 0, 0);
	XFreePixmap(XtDisplay(w), scribble);
	XFreeGC(XtDisplay(w), gc);
/* update plugin positions */
	DoLayout(rtw);

	/* clear the timeout */
	rtw->html.timeout = None;
}

static void Redisplay(Widget w, XEvent *event, Region r) {
	MwHtmlWidget tw = (MwHtmlWidget)w;
	if (tw->html.timeout) return;	/* already set */
	if (tw->html.delay) {
		tw->html.timeout = XtAppAddTimeOut(
			XtWidgetToApplicationContext(w),
			tw->html.delay, do_redisplay, (XtPointer)w);
	} else {
		do_redisplay((XtPointer)w, NULL);
	}
}

static Boolean SetValues(Widget current, Widget request, Widget new,
	ArgList args, Cardinal *nargs) {
	MwHtmlWidget hw = (MwHtmlWidget)new;
	MwHtmlWidget cw = (MwHtmlWidget)current;
	Boolean do_redisplay = True;

	if (hw->html.url != NULL && hw->html.url != cw->html.url) {
		hw->html.url = x_resolve_url(cw->html.url, hw->html.url);
		MwFree(cw->html.url);
		free_text(hw);
	}
	if (hw->html.ob == NULL || hw->html.ob != cw->html.ob) {
		free_text(hw);
	}
	if (do_redisplay) Redisplay(new, NULL, None);
	return False;
}

static Boolean ConstraintSetValues(Widget current, Widget request,
	Widget new, ArgList args, Cardinal *num_args) {
	return False;	/* no, I don't know what this does */
}

/*
 * Do a layout, actually assigning positions.
 *
 * This function uses a callback from the application to get the
 * position of each plugin. In the case of PW, that callback uses
 * richtext_char2coords. See pw/window.c.
 */

static void DoLayout(MwHtmlWidget hw) {
	MwHtmlConstraints hc;
	Widget child;
	int i;
	int b, x, y, w, h;
	object_box *ob;

	for (i = 0; i < hw->composite.num_children; i++) {
		child = hw->composite.children[i];
		if (!child->core.managed) continue;

		hc = (MwHtmlConstraints)child->core.constraints;
		ob = hc->html.ob;
		if (ob == NULL) continue;	/* e.g. menu shells */
		absolute_position(ob, &x, &y);
		b = child->core.border_width;
		x -= hw->html.top_col;
		y -= hw->html.top_row;
		w = ob->w - 2 * b;
		h = ob->h - 2 * b;
		XtConfigureWidget(child, x, y, w, h, b);
	}
}

/*
 * Actually layout the table
 */

static void Resize(Widget w) {
	MwHtmlWidget hw = (MwHtmlWidget)w;
	free_text(hw);
	DoLayout(hw);
}

/*
 * Geometry Manager
 */

static XtGeometryResult GeometryManager(Widget w,
	XtWidgetGeometry *request, XtWidgetGeometry *reply) {
	return XtGeometryYes;
}

static void ChangeManaged(Widget w) {
	DoLayout((MwHtmlWidget)w);
}

int MwHtmlLookupString(Widget w, XEvent *event, char *buf,
	int bufsiz, KeySym *keysym) {
#ifdef HAVE_XCREATEIC
	Status status;
	return XmbLookupString(((MwHtmlWidget)w)->html.xic,
		(XKeyEvent *)event, (char *)buf, bufsiz,
		keysym, &status);
#else
	return XLookupString((XKeyEvent *)event, (char *)buf, bufsiz,
		keysym, NULL);
#endif
}

void MwHtmlSetZoom(Widget w, float zoom) {
	MwHtmlWidget rw = (MwHtmlWidget)w;

	if (zoom < .1) zoom = .1;
	if (zoom > 10) zoom = 10;
	if (rw->html.zoom != zoom) {
		rw->html.zoom = zoom;
		Redisplay(w, NULL, None);
	}
}
