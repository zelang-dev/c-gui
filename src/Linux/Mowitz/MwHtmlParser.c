/*
This module parses HTML and anything that looks like it, i.e. is
composed of <tags> and text. That means it should be able to read
XML and similar as well. It doesn't interpret what it sees at all.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <Linux/Mowitz/MwUtils.h>
#include <Linux/Mowitz/MwFormat.h>
#include <Linux/Mowitz/MwHtmlParser.h>

static MwFmt fmt0 = {
        "New Century Schoolbook",    /* font family */
        120,            /* size in decipoints */
        0,              /* bold */
        0,              /* italic */
        0,              /* underline */
        0,              /* strikethrough */
        "black",        /* foreground */
        "white",        /* background */
        0,              /* borders */
        MW_VADJ_CENTER, /* vertical adjust */
        MW_HADJ_LEFT,   /* horizontal adjust */
        0};             /* style */

static MwFmt fmt_h1 = {"Helvetica", 240, 1, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};
static MwFmt fmt_h2 = {"Helvetica", 180, 1, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};
static MwFmt fmt_h3 = {"Helvetica", 160, 1, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};
static MwFmt fmt_h4 = {"Helvetica", 140, 1, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};
static MwFmt fmt_h5 = {"Helvetica", 120, 1, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};
static MwFmt fmt_h6 = {"Helvetica", 100, 1, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};
static MwFmt fmt_plain = {"Courier", 120, 0, 0, 0, 0, "black", "white",
                        0, MW_VADJ_CENTER, MW_HADJ_LEFT, 0};

enum {LIST_NONE=0, LIST_UL, LIST_OL, LIST_DL};

typedef struct hstate {
	int quiet;			/* don't output anything */
        int pre;                        /* preformatted, default 0 = false */
        int list;                       /* list mode, default LIST_NONE */
        int ll;                         /* list level, starts with 1 */
        int indent;                     /* indentation level, default 0 */
        int fmt;                        /* format */
} hstate;

typedef struct html_info {
	char *b;
	int n, nmax;
	hstate h;
	struct tagstack *ts;
	struct {
		char *action;
		char *method;
	} form;
	object_box *ob, /* *tail,*/ *parent;
} html_info;

typedef struct tagstack {
	char *tag;
	char *text;
	void (*atend)(struct html_info *);
	hstate h;
	struct tagstack *next;
} tagstack;

static struct {
	char *name;
	int type;
} input_types[] = {
	{"TEXT", MW_HTML_INPUT_TEXT},
	{"PASSWORD", MW_HTML_INPUT_PASSWORD},
	{"CHECKBOX", MW_HTML_INPUT_CHECKBOX},
	{"RADIO", MW_HTML_INPUT_RADIO},
	{"SUBMIT", MW_HTML_INPUT_SUBMIT},
	{"IMAGE", MW_HTML_INPUT_IMAGE},
	{"RESET", MW_HTML_INPUT_RESET},
	{"BUTTON", MW_HTML_INPUT_BUTTON},
	{"HIDDEN", MW_HTML_INPUT_HIDDEN},
	{"FILE", MW_HTML_INPUT_FILE},
	{NULL, 0}};

/* convert string in place */
static void unquote(char *p)
{
	int state = 0;
	int c;
	char *q, *s;

	q = s = p;
	while (*p) {
		switch (state) {
		case 0:
			if (*p == '&') {
				state = 1;
				s = p+1;
			} else {
				*q++ = *p;
			}
			p++;
			break;
		case 1:
			if (*p == ';') {
				*p = '\0';
				c = MwFromCchar(s);
				if (c == -1) {	/* no match found */
					sprintf(q, "&%s;", s);
					q += strlen(q);
				} else {
					*q++ = c;
				}
				state = 0;
			}
			p++;
			break;
		}
	}
	*q = '\0';
}


static MwHtmlNode *last_node(MwHtmlNode *h)
{
	if (h == NULL) return NULL;
	if (h->next == NULL) return h;
	return last_node(h->next);
}

static void grow_b(html_info *p, int n)
{
	if (n >= p->nmax) {
		p->nmax = n+1000;
		p->b = MwRealloc(p->b, p->nmax+1);
	}
}

static void clear_b(html_info *p)
{
	p->n = 0;
	p->b[0] = '\0';
}

static void emitchar(html_info *p, int c)
{
	grow_b(p, p->n+1);
	p->b[p->n++] = c;
	p->b[p->n] = '\0';
}

/* find an ancestor of specified type */
static object_box *find_parent(object_box *ob, int type)
{
	if (ob == NULL) {
		return ob;
	} else if (ob->type == type) {
		return ob;
	}
	return find_parent(ob->parent, type);
}

/* add an object as the last child of the current parent */
static object_box *add_object_box(html_info *p, int type, void *data)
{
	object_box *parent = p->parent;
	object_box *ob = MwMalloc(sizeof *ob);
	ob->type = type;
	ob->data = data;
	ob->child = ob->last = ob->next = NULL;
	ob->parent = parent;
	if (parent->child == NULL) parent->child = ob;
	else parent->last->next = ob;
	parent->last = ob;
	return ob;
}

static void newline(html_info *p)
{
	add_object_box(p, MW_HTML_NEWLINE, NULL);
}

static void emitspace(html_info *p)
{
	char *q, *r;

	if (p->n == 0) return;
	if (p->h.quiet == 0) {
		if (p->h.pre) {
			q = p->b;
			while ((r = strchr(q, '\n'))) {
				*r = '\0';
				add_object_box(p, MW_HTML_WORDPART,
						MwRcMakerich(q, p->h.fmt));
				newline(p);
				q = r+1;
			}
			if (*q) add_object_box(p, MW_HTML_WORDPART,
						MwRcMakerich(q, p->h.fmt));
		} else {
			add_object_box(p, MW_HTML_SPACE, MwRcMakerich(p->b, p->h.fmt));
		}
	}
	clear_b(p);
}

static void tag_a(html_info *);
static void tag_address(html_info *);
static void tag_align(html_info *);
static void tag_area(html_info *);
static void tag_b(html_info *);
static void tag_base(html_info *);
static void tag_basefont(html_info *);
static void tag_big(html_info *);
static void tag_blockquote(html_info *);
static void tag_body(html_info *);
static void tag_br(html_info *);
static void tag_button(html_info *);
static void tag_center(html_info *);
static void tag_cite(html_info *);
static void tag_code(html_info *);
static void tag_dd(html_info *);
static void tag_dir(html_info *);
static void tag_div(html_info *);
static void tag_dl(html_info *);
static void tag_dt(html_info *);
static void tag_em(html_info *);
static void tag_font(html_info *);
static void tag_form(html_info *);
static void tag_h1(html_info *);
static void tag_h2(html_info *);
static void tag_h3(html_info *);
static void tag_h4(html_info *);
static void tag_h5(html_info *);
static void tag_h6(html_info *);
static void tag_head(html_info *);
static void tag_hr(html_info *);
static void tag_html(html_info *);
static void tag_i(html_info *);
static void tag_img(html_info *);
static void tag_input(html_info *);
static void tag_li(html_info *);
static void tag_map(html_info *);
static void tag_menu(html_info *);
static void tag_ol(html_info *);
static void tag_optgroup(html_info *);
static void tag_option(html_info *);
static void tag_p(html_info *);
static void tag_pre(html_info *);
static void tag_q(html_info *);
static void tag_s(html_info *);
static void tag_samp(html_info *);
static void tag_script(html_info *);
static void tag_select(html_info *);
static void tag_small(html_info *);
static void tag_strike(html_info *);
static void tag_strong(html_info *);
static void tag_style(html_info *);
static void tag_sub(html_info *);
static void tag_sup(html_info *);
static void tag_table(html_info *);
static void tag_td(html_info *);
static void tag_textarea(html_info *);
static void tag_th(html_info *);
static void tag_tr(html_info *);
static void tag_tt(html_info *);
static void tag_u(html_info *);
static void tag_ul(html_info *);
static void tag_unknown(html_info *);
static void tag_var(html_info *);

static struct {
	char *name;
	void (*action)(html_info *);
} tag[] = {
	{"A", tag_a},
	{"ADDRESS", tag_address},
	{"ALIGN", tag_align},
        {"AREA", tag_area},
        {"B", tag_b},
	{"BASE", tag_base},
	{"BASEFONT", tag_basefont},
	{"BIG", tag_big},
	{"BLOCKQUOTE", tag_blockquote},
        {"BODY", tag_body},
        {"BR", tag_br},
	{"BUTTON", tag_button},
	{"CENTER", tag_center},
	{"CITE", tag_cite},
	{"CODE", tag_code},
        {"DD", tag_dd},
	{"DIR", tag_dir},
	{"DIV", tag_div},
        {"DL", tag_dl},
        {"DT", tag_dt},
	{"EM", tag_em},
	{"FONT", tag_font},
	{"FORM", tag_form},
        {"H1", tag_h1},
        {"H2", tag_h2},
        {"H3", tag_h3},
        {"H4", tag_h4},
        {"H5", tag_h5},
        {"H6", tag_h6},
        {"HEAD", tag_head},
        {"HR", tag_hr},
        {"HTML", tag_html},
        {"I", tag_i},
        {"IMG", tag_img},
	{"INPUT", tag_input},
        {"LI", tag_li},
        {"MAP", tag_map},
	{"MENU", tag_menu},
        {"OL", tag_ol},
	{"OPTGROUP", tag_optgroup},
	{"OPTION", tag_option},
        {"P", tag_p},
        {"PRE", tag_pre},
	{"Q", tag_q},
	{"S", tag_s},
	{"SAMP", tag_samp},
	{"SCRIPT", tag_script},
	{"SELECT", tag_select},
	{"SMALL", tag_small},
	{"STRIKE", tag_strike},
	{"STRONG", tag_strong},
	{"STYLE", tag_style},
	{"SUB", tag_sub},
	{"SUP", tag_sup},
        {"TABLE", tag_table},
        {"TD", tag_td},
	{"TEXTAREA", tag_textarea},
        {"TH", tag_th},
        {"TR", tag_tr},
	{"TT", tag_tt},
	{"U", tag_u},
        {"UL", tag_ul},
	{"VAR", tag_var},
	{NULL, tag_unknown}
};

static void indent(html_info *p, int n)
{
	p->h.indent += n;
	add_object_box(p, MW_HTML_INDENT, (void *)(p->h.indent));
}

/* the indentation level is automatically reset after a list, but we
   need this to tell the widget about the new value */
static void unindent(html_info *p)
{
	indent(p, 0);
	newline(p);
}

/* Collect text located in children of an object box. Used by elements
   such as option and textarea. */
static char *collect_text(object_box *ob)
{
	object_box *ob1;
	char *q, *r;
	int n, n1;
	r = MwStrdup("");
	n = 0;
	for (ob1 = ob->child; ob1; ob1 = ob1->next) {
		if (ob1->type == MW_HTML_WORDPART ||
		    ob1->type == MW_HTML_SPACE) {
			q = MwRcMakeplain(ob1->data);
			printf("%s", q);
			n1 = strlen(q);
			r = MwRealloc(r, n+n1+1);
			strcpy(r+n, q);
			n += n1;
			MwFree(q);
		} else if (ob1->type == MW_HTML_NEWLINE) {
			printf("\n");
			r = MwRealloc(r, n+1+1);
			strcpy(r+n, "\n");
			n++;
		}
	}
	return r;
}

static void end_anchor(html_info *p)
{
	add_object_box(p, MW_HTML_END_A, NULL);
}

/* Anchor tag */
static void tag_a(html_info *p)
{
	MwHtmlAnchor *a = MwMalloc(sizeof *a);
	MwFmt fmt;
	a->href = MwHtmlGetValue("HREF", p->ts->text);
	a->name = MwHtmlGetValue("NAME", p->ts->text);
	if (a->href) {
		MwDecodeFormat(p->h.fmt, ~0, &fmt);
		fmt.uline = 1;
		fmt.fg = "blue";
		p->h.fmt = MwEncodeFormat(~0, &fmt);
	}
	add_object_box(p, MW_HTML_BEGIN_A, a);
	p->ts->atend = end_anchor;
	/* don't do the anchor right now */
}

/* Address tag. Only changes font */
static void tag_address(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.italic = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_align(html_info *p)
{
	newline(p);
	p->ts->atend = newline;
}

/* Treat these as <a> */
static void tag_area(html_info *p)
{
	;
}

static void tag_b(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.bold = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_base(html_info *p)
{
	char *base = MwHtmlGetValue("HREF", p->ts->text);
	if (base) add_object_box(p, MW_HTML_BASE, base);
}

static void tag_basefont(html_info *p)
{
	MwFmt fmt;
	int i, n;
	char *size = MwHtmlGetValue("SIZE", p->ts->text);
	char *color = MwHtmlGetValue("COLOR", p->ts->text);
	char *face = MwHtmlGetValue("FACE", p->ts->text);
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	if (size) {
		if (size[0] == '+') {
			n = atoi(size+1);
			for (i = 0; i < n; i++) fmt.size *= 1.2;
		} else if (size[0] == '-') {
			n = atoi(size+1);
			for (i = 0; i < n; i++) fmt.size *= 0.8;
		} else {
			n = atoi(size);
			fmt.size = 120;
			if (n < 3) {
				for (i = 3; i > n; i--) fmt.size *= 0.8;
			} else {
				for (i = 3; i < n; i++) fmt.size *= 1.2;
			}
		}
	}
	if (color) {
		fmt.fg = color;
	}
	if (face) {
		char *p = strchr(face, ',');
		if (p) *p = '\0';
		fmt.family = face;
	}
	p->h.fmt = MwEncodeFormat(~0, &fmt);
	MwFree(size);
	MwFree(color);
	MwFree(face);
}

static void tag_big(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.size = 160;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_blockquote(html_info *p)
{
	indent(p, 1);
	newline(p);
	p->ts->atend = unindent;
}

/* Check for background values et al */
static void tag_body(html_info *p)
{
	p->h.quiet = 0;
}

/* Line break */
static void tag_br(html_info *p)
{
	newline(p);
}

/* a button is treated exactly like a submit or reset control */
static void tag_button(html_info *p)
{
	int i;
	MwHtmlInput *a;
	char *type = MwHtmlGetValue("TYPE", p->ts->text);
	if (type == NULL) type = MwStrdup("submit");
	for (i = 0; input_types[i].name; i++)
		if (!MwStrcasecmp(input_types[i].name, type)) break;
	MwFree(type);
	if (!input_types[i].name) return;
	a = MwMalloc(sizeof *a);
	a->type = input_types[i].type;
	a->name = MwHtmlGetValue("NAME", p->ts->text);
	a->value = MwHtmlGetValue("VALUE", p->ts->text);
	if (a->value == NULL) a->value = MwStrdup("");
	a->action = p->form.action;
	a->method = p->form.method;
	add_object_box(p, MW_HTML_INPUT, a);
}

static void tag_center(html_info *p)
{
	newline(p);
	p->ts->atend = newline;
}

static void tag_cite(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.italic = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_code(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.family = "Courier";
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

/* description in description list */
static void tag_dd(html_info *p)
{
	add_object_box(p, MW_HTML_INDENT, (void *)(p->h.indent));
	newline(p);
}

static void tag_dir(html_info *p)
{
	p->h.list = LIST_UL;
	p->h.ll = 1;
	indent(p, 1);
	newline(p);
	p->ts->atend = unindent;
}

static void tag_div(html_info *p)
{
	newline(p);
	p->ts->atend = newline;
}

/* start description list */
static void tag_dl(html_info *p)
{
	p->h.list = LIST_DL;
	p->h.ll = 1;
	indent(p, 1);
	newline(p);
	p->ts->atend = unindent;
}

/* tag in description list */
static void tag_dt(html_info *p)
{
	add_object_box(p, MW_HTML_INDENT, (void *)(p->h.indent-1));
	newline(p);
}

static void tag_em(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.italic = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_font(html_info *p)
{
	MwFmt fmt;
	int i, n;
	char *size = MwHtmlGetValue("SIZE", p->ts->text);
	char *color = MwHtmlGetValue("COLOR", p->ts->text);
	char *face = MwHtmlGetValue("FACE", p->ts->text);
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	if (size) {
		if (size[0] == '+') {
			n = atoi(size+1);
			for (i = 0; i < n; i++) fmt.size *= 1.2;
		} else if (size[0] == '-') {
			n = atoi(size+1);
			for (i = 0; i < n; i++) fmt.size *= 0.8;
		} else {
			n = atoi(size);
			fmt.size = 120;
			if (n < 3) {
				for (i = 3; i > n; i--) fmt.size *= 0.8;
			} else {
				for (i = 3; i < n; i++) fmt.size *= 1.2;
			}
		}
	}
	if (color) {
		fmt.fg = color;
	}
	if (face) {
		char *p = strchr(face, ',');
		if (p) *p = '\0';
		fmt.family = face;
	}
	p->h.fmt = MwEncodeFormat(~0, &fmt);
	MwFree(size);
	MwFree(color);
	MwFree(face);
}

static void close_form(html_info *p)
{
	/* this may be broken: potential memory leak */
	p->form.action = NULL;
}

static void tag_form(html_info *p)
{
	MwFree(p->form.action);
	MwFree(p->form.method);
	p->form.action = MwHtmlGetValue("ACTION", p->ts->text);
	p->form.method = MwHtmlGetValue("METHOD", p->ts->text);
	if (p->form.action == NULL) {
		printf("Form without action\n");
		return;
	}
	if (p->form.method == NULL) p->form.method = MwStrdup("get");
	p->ts->atend = close_form;
}

static void tag_h1(html_info *p)
{
	newline(p);
	p->h.fmt = MwEncodeFormat(~0, &fmt_h1);
	p->ts->atend = newline;
}

static void tag_h2(html_info *p)
{
	newline(p);
	p->h.fmt = MwEncodeFormat(~0, &fmt_h2);
	p->ts->atend = newline;
}

static void tag_h3(html_info *p)
{
	newline(p);
	p->h.fmt = MwEncodeFormat(~0, &fmt_h3);
	p->ts->atend = newline;
}

static void tag_h4(html_info *p)
{
	newline(p);
	p->h.fmt = MwEncodeFormat(~0, &fmt_h4);
	p->ts->atend = newline;
}

static void tag_h5(html_info *p)
{
	newline(p);
	p->h.fmt = MwEncodeFormat(~0, &fmt_h5);
	p->ts->atend = newline;
}

static void tag_h6(html_info *p)
{
	newline(p);
	p->h.fmt = MwEncodeFormat(~0, &fmt_h6);
	p->ts->atend = newline;
}

/* ignore this tag and all children */
/* Set a flag that this is to be ignored */
static void tag_head(html_info *p)
{
	p->h.quiet = 1;
}

/* Horizontal ruler */
static void tag_hr(html_info *p)
{
	newline(p);
	add_object_box(p, MW_HTML_HR, NULL);
	newline(p);
}

/* this tag doesn't do anything per se */
static void tag_html(html_info *p)
{
	;
}

static void tag_i(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.italic = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

/* Image tag. Ignored until we have somewhere to show pictures. */
static void tag_img(html_info *p)
{
	char b[1024];
	MwHtmlImage *a = MwMalloc(sizeof *a);
	char *q = MwHtmlGetValue("ALT", p->ts->text);
	if (q == NULL) strcpy(b, "[INLINE]");
	else strcpy(b, q);
	a->src = MwHtmlGetValue("SRC", p->ts->text);
	a->alt = MwHtmlGetValue("ALT", p->ts->text);
	MwFree(q);
	add_object_box(p, MW_HTML_IMAGE, a);
}

static void tag_input(html_info *p)
{
	int i;
	MwHtmlInput *a;
	char *type = MwHtmlGetValue("TYPE", p->ts->text);
	if (type == NULL) return;		/* no type at all */
	for (i = 0; input_types[i].name; i++)
		if (!MwStrcasecmp(input_types[i].name, type)) break;
	MwFree(type);
	if (!input_types[i].name) return;	/* bogus type */
	a = MwMalloc(sizeof *a);
	a->type = input_types[i].type;
	a->name = MwHtmlGetValue("NAME", p->ts->text);
	a->value = MwHtmlGetValue("VALUE", p->ts->text);
	if (a->value == NULL) a->value = MwStrdup("");
	a->action = p->form.action;	/* doubles as ID */
	a->method = p->form.method;
	add_object_box(p, MW_HTML_INPUT, a);
}

/* this is because any </li> tags would otherwise reset the counter */
static void fix_listlevel(html_info *p)
{
	p->h.ll++;
}

/* start a new line. Should also fiddle with indentation */
static void tag_li(html_info *p)
{
	newline(p);
	switch (p->h.list) {
	case LIST_UL:
	add_object_box(p, MW_HTML_UBULLET, (void *)'*');
		break;
	case LIST_OL:
	add_object_box(p, MW_HTML_OBULLET, (void *)p->h.ll);
		break;
	default:
	add_object_box(p, MW_HTML_UBULLET, (void *)' ');
		break;
	}
	p->h.ll++;
	p->ts->atend = fix_listlevel;
}

/* Not much to do here... */
static void tag_map(html_info *p)
{
	;
}

static void tag_menu(html_info *p)
{
	p->h.list = LIST_UL;
	p->h.ll = 1;
	indent(p, 1);
	newline(p);
	p->ts->atend = unindent;
}

static void tag_ol(html_info *p)
{
	p->h.list = LIST_OL;
	p->h.ll = 1;
	indent(p, 1);
	newline(p);
	p->ts->atend = unindent;
}

static void tag_optgroup(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_SELECT);
	MwHtmlInput *a;
	if (ob == NULL) {
printf("Big problem: optgroup must be child of select\n");
	} else {
		p->parent = ob;
	}
	a = MwMalloc(sizeof *a);
	a->name = MwHtmlGetValue("LABEL", p->ts->text);
	if (a->name == NULL) return;
	p->parent = add_object_box(p, MW_HTML_OPTGROUP, a);
}

static void tag_option(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_SELECT);
	MwHtmlInput *a;
	if (ob == NULL) {
printf("Big problem: option must be child of select\n");
	} else {
		p->parent = ob;
	}
	a = MwMalloc(sizeof *a);
	a->name = MwHtmlGetValue("LABEL", p->ts->text);
	a->value = MwHtmlGetValue("VALUE", p->ts->text);
	p->parent = add_object_box(p, MW_HTML_OPTION, a);
}

/* Paragraph break */
static void tag_p(html_info *p)
{
	newline(p);
}

/* Set a flag that this is preformatted */
static void tag_pre(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.family = "Courier";
	p->h.fmt = MwEncodeFormat(~0, &fmt);
	p->h.pre = 1;
	newline(p);
	p->ts->atend = newline;
}

static void q_closer(html_info *p)
{
	add_object_box(p, MW_HTML_WORDPART, MwRcMakerich("''", p->h.fmt));
}

/* This should produce different quotation marks depending on locale. */
static void tag_q(html_info *p)
{
	add_object_box(p, MW_HTML_WORDPART, MwRcMakerich("``", p->h.fmt));
	p->ts->atend = q_closer;
}

static void tag_s(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.strike = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_samp(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.family = "Courier";
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

/* Set a flag that this is to be ignored */
static void tag_script(html_info *p)
{
	p->h.quiet = 1;
}

static void close_select(html_info *p)
{
	MwHtmlInput *a;
	object_box *ob = find_parent(p->parent, MW_HTML_SELECT);
	object_box *ob1;
	char *q;
	char **list = NULL;
	int nlist = 0;
	int i;

	if (ob == NULL) {
printf("Big problem: can't find beginning of select\n");
	} else {
		p->parent = ob->parent;
	}
	ob->type = MW_HTML_INPUT;
printf("FIXME: should free children\n");
	for (ob1 = ob->child; ob1; ob1 = ob1->next) {
		if (ob1->type == MW_HTML_OPTION) {
			a = ob1->data;
			if (a->value) {
				q = a->value;
			} else {
				q = collect_text(ob1);
			}
			list = MwRealloc(list, (nlist+1)*sizeof *list);
			list[nlist] = q;
			nlist++;
		}
	}
	list = MwRealloc(list, (nlist+1)*sizeof *list);
	list[nlist] = NULL;
	for (i = 0; i < nlist; i++)
	a = ob->data;
	a->value = (char *)list;
	ob->child = ob->last = NULL;
}

static void tag_select(html_info *p)
{
	MwHtmlInput *a;
	char *m = MwHtmlGetValue("MULTIPLE", p->ts->text);
	a = MwMalloc(sizeof *a);

	if (m) a->type = MW_HTML_INPUT_SELECT2;
	else a->type = MW_HTML_INPUT_SELECT;
	MwFree(m);
	a->name = MwHtmlGetValue("NAME", p->ts->text);
	a->action = p->form.action;
	a->method = p->form.method;
	a->value = NULL;
	p->parent = add_object_box(p, MW_HTML_SELECT, a);
	p->ts->atend = close_select;
}

static void tag_small(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.size = 100;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_strike(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.strike = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_strong(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.bold = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

/* Set a flag that this is to be ignored */
static void tag_style(html_info *p)
{
	p->h.quiet = 1;
}

static void tag_sub(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.vadj = MW_VADJ_BOTTOM;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_sup(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.vadj = MW_VADJ_TOP;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void close_table(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_TABLE);
	if (ob == NULL) {
printf("Big problem: can't find beginning of table\n");
	} else {
		p->parent = ob->parent;
	}
	p->h.quiet = 0;
	add_object_box(p, MW_HTML_NEWLINE, NULL);
}

static void tag_table(html_info *p)
{
	add_object_box(p, MW_HTML_NEWLINE, NULL);
	p->parent = add_object_box(p, MW_HTML_TABLE, NULL);
	p->h.quiet = 1;
	p->ts->atend = close_table;
}

/* Table cell */
static void tag_td(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_ROW);
	if (ob == NULL) {
printf("Big problem: CELL must be child of ROW\n");
	} else {
		p->parent = ob;
	}
	p->h.quiet = 0;
	p->parent = add_object_box(p, MW_HTML_CELL, NULL);
}

static void close_textarea(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_TEXTAREA);
	MwHtmlInput *a;
	if (ob == NULL) {
printf("Big problem: can't find beginning of textarea\n");
	} else {
		p->parent = ob->parent;
	}
	ob->type = MW_HTML_INPUT;
printf("FIXME: should free children\n");
	a = ob->data;
	a->value = collect_text(ob);
	ob->child = ob->last = NULL;
	add_object_box(p, MW_HTML_NEWLINE, NULL);
}

static void tag_textarea(html_info *p)
{
	MwHtmlInput *a = MwMalloc(sizeof *a);
	a->type = MW_HTML_INPUT_TEXTAREA;
	a->name = MwHtmlGetValue("NAME", p->ts->text);
	if (a->name == NULL) a->name = MwStrdup("textarea");
	a->value = NULL;
	a->action = p->form.action;
	a->method = p->form.method;
	add_object_box(p, MW_HTML_NEWLINE, NULL);
	p->h.pre = 1;
	p->parent = add_object_box(p, MW_HTML_TEXTAREA, a);
	p->ts->atend = close_textarea;
}

/* Table header */
static void tag_th(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_ROW);
	if (ob == NULL) {
printf("Big problem: CELL must be child of ROW\n");
	} else {
		p->parent = ob;
	}
	p->parent = add_object_box(p, MW_HTML_CELL, NULL);
	p->h.quiet = 1;
}

/* Table row */
static void tag_tr(html_info *p)
{
	object_box *ob = find_parent(p->parent, MW_HTML_TABLE);
	if (ob == NULL) {
printf("Big problem: ROW must be child of TABLE\n");
	} else {
		p->parent = ob;
	}
	p->parent = add_object_box(p, MW_HTML_ROW, NULL);
}

static void tag_tt(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.family = "Courier";
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_u(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.uline = 1;
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

static void tag_ul(html_info *p)
{
	p->h.list = LIST_UL;
	p->h.ll = 1;
	indent(p, 1);
	newline(p);
	p->ts->atend = unindent;
}

static void tag_var(html_info *p)
{
	MwFmt fmt;
	MwDecodeFormat(p->h.fmt, ~0, &fmt);
	fmt.family = "Courier";
	p->h.fmt = MwEncodeFormat(~0, &fmt);
}

/* Handles tags we don't have handlers for. Ignore the tag. */
static void tag_unknown(html_info *p)
{
printf("tag_unknown() ignores tag '%s'\n", p->ts->tag);
}

static void emittag(html_info *p)
{
	int i;
	void (*action)(html_info *p);
	char *s, *t;
	tagstack *ts;
	if (p->n == 0) return;

	unquote(p->b);
	p->n = strlen(p->b);
	t = p->b+1;		/* skip leading < */
	p->b[p->n-1] = '\0';	/* strip trailing > */
	for (s = t; *s && !isspace(*s); s++);
	if (s) *s++ = '\0';	/* split tag and parameters */

	if (t[0] == '/') {	/* end tag, look for match in stack */
		t++;
		for (ts = p->ts; ts; ts = ts->next) {
			if (!MwStrcasecmp(ts->tag, t)) {
				while (p->ts && MwStrcasecmp(p->ts->tag, t)) {
					ts = p->ts;
					p->ts = ts->next;
					MwFree(ts->tag);
					MwFree(ts->text);
				}
			}
		}
		if (p->ts == NULL) {
printf("Warning: bogus closer '%s'\n", t);
			clear_b(p);
			return;
		}
		ts = p->ts;
		p->ts = ts->next;
		p->h = ts->h;
		MwFree(ts->tag);
		MwFree(ts->text);
		action = ts->atend;
		MwFree(ts);
		if (action) (*action)(p);

		clear_b(p);
		return;
	}

	/* we're here, so it's a start tag */
	ts = MwMalloc(sizeof(*ts));
	if (s == NULL) s = "";
	ts->tag = MwStrdup(t);
	ts->text = MwStrdup(s);
	ts->atend = NULL;
	ts->h = p->h;
	ts->next = p->ts;
	p->ts = ts;
	for (i = 0; tag[i].name; i++) {
		if (!MwStrcasecmp(tag[i].name, t)) break;
	}
	action = tag[i].action;

	(*action)(p);
	clear_b(p);
}

static void emitword(html_info *p)
{
	if (p->n == 0) return;
	if (p->h.quiet == 0) {
		unquote(p->b);
		add_object_box(p, MW_HTML_WORDPART, MwRcMakerich(p->b, p->h.fmt));
	}
	clear_b(p);
}


/* Reference counted cache */
struct ob_cache {
	url_info *ui;
	object_box *ob;
	int ref;
	struct ob_cache *next;
} *oc;

void MwHtmlFree(object_box *o1)
{
	struct ob_cache *o;

	if (o1 == NULL) return;

	for (o = oc; o->ob != o1; o = o->next);
	if (o) {
		o->ref--;
		return;		/* don't actually free anything */
	}
}

static void dump_tree(object_box *ob, int level)
{
	int i;

	if (ob == NULL) return;
	for (i = 0; i < level; i++) printf("   ");
	printf("<%d>\n", ob->type);
	dump_tree(ob->child, level+1);
	dump_tree(ob->next, level);
}

#if 0	/* not done yet */
static void flush_space(html_info *p)
{
	if (spacen) printf(
#endif

/* The relevant fields in ui are the local file name, the size and
   the header string. These are filled in by load_url.
*/
object_box *MwHtmlParse(char *url)
{
	enum {START = 0, SPACE, NEWLINE1, NEWLINE2, ENDTAG,
		BEGIN_TAG, STARTTAG, NEWLINE3, WORDPART, END} state;
	int error;
	int c;
	html_info p;
	url_info *ui;
	struct ob_cache *o, *o1;
	FILE *fp;
	char *t, *major, *minor, b[100];
#if 0
	char *html, *q, *r;
#endif

	for (o = oc; o; o = o->next) {
		if (!strcmp(url, o->ui->url)) {
			o->ref++;
			return o->ob;
		}
	}
	ui = load_url(url);
	if (ui == NULL) return NULL;

	fp = fopen(ui->local, "r");
	if (fp == NULL) {
		free_url(ui);
		return NULL;
	}

	p.n = p.nmax = 0;
	p.b = NULL;
	grow_b(&p, 1);
	p.ts = NULL;
	p.h.pre = 0;
	p.h.quiet = 0;
	p.h.indent = 0;
	p.h.list = LIST_NONE;
	p.h.ll = 0;
	p.h.fmt = MwEncodeFormat(~0, &fmt0);
	/* we must seed the tree with a toplevel parent */
	p.parent = p.ob = MwMalloc(sizeof *p.ob);
	p.ob->type = MW_HTML_CELL;
	p.ob->data = NULL;	/* CELLs have no data */
	p.ob->next = NULL;	/* will remain NULL */
	p.ob->parent = NULL;	/* there is nothing above us */
	p.ob->child = NULL;	/* will be added later */
	p.form.action = p.form.method = NULL;

	error = 0;

	t = ui->header;
	while (t) {
		if (!MwStrncasecmp(t, "Content-Type:", 13)) break;
			t = strchr(t, '\n');	/* will magically skip \r */
		if (t) t++;
	}
	if (t) {
		int i;
		t += 13;
		while (*t && isspace(*t)) t++;
		for (i = 0; t[i] && t[i] != '\r' && t[i] != '\n'; i++) b[i] = t[i];
		b[i] = '\0';
		major = b;
		minor = strchr(b, '/');
		if (minor) *minor++ = '\0';
		else minor = "unknown";
	} else {
		major = "text";
		minor = "html";
	}

	/* an image is a single box */
	if (!MwStrcasecmp(major, "image")) {
		MwHtmlImage *a = MwMalloc(sizeof *a);
		a->src = MwStrdup(ui->url);
		a->alt = NULL;
		add_object_box(&p, MW_HTML_IMAGE, a);
		goto Reverse;
	}

	/* each line is one wordpart */
	if (!MwStrcasecmp(major, "text") && !MwStrcasecmp(minor, "plain")) {
		error = 0;
		p.h.fmt = MwEncodeFormat(~0, &fmt_plain);
		while ((c = fgetc(fp)) != EOF) {
			if (c == '\n') {
				add_object_box(&p, MW_HTML_WORDPART,
					MwRcMakerich(p.b, p.h.fmt));
				clear_b(&p);
				newline(&p);
			} else {
				emitchar(&p, c);
			}
		}
		emitword(&p);
		goto Reverse;
	}

	/* assume it's html then */
	state = START;
	error = 0;
#if 1
	while (state != END) {
		c = fgetc(fp);
		if (c == EOF) c = '\0';
/*printf("state = %d, c = '%c'\n", state, c);*/
		switch (state) {
		case START:
			if (c == '\0') {
				state = END;
			} else if (c == '\n') {
				/* must ignore newline before end tag */
				state = NEWLINE1;
			} else if (c == '<') {
				emitchar(&p, c);
				state = BEGIN_TAG;
			} else if (isspace(c)) {
				emitchar(&p, c);
				state = SPACE;
			} else {
				emitchar(&p, c);
				state = WORDPART;
			}
			break;
		case SPACE:
			if (c == '\0') {
				emitspace(&p);
				state = END;
			} else if (c == '\n') {
				state = NEWLINE1;
			} else if (c == '<') {
				emitspace(&p);
				emitchar(&p, c);
				state = BEGIN_TAG;
			} else if (isspace(c)) {
				emitchar(&p, c);
			} else {
				emitspace(&p);
				emitchar(&p, c);
				state = WORDPART;
			}
			break;
		case NEWLINE1:
			if (c == '\0') {
				emitspace(&p);
				state = END;
			} else if (c == '<') {
				state = NEWLINE2;
			} else if (c == '\n') {
				emitchar(&p, '\n');
			} else if (isspace(c)) {
				emitchar(&p, '\n');
				emitchar(&p, c);
				state = SPACE;
			} else {
				emitchar(&p, '\n');
				emitspace(&p);
				emitchar(&p, c);
				state = WORDPART;
			}
			break;
		case NEWLINE2:
			if (c == '\0') {
				emitchar(&p, '\n');
				emitspace(&p);
				emitchar(&p, '<');
				emitword(&p);
				state = END;
			} else if (c == '/') {
				emitchar(&p, '\n');
				emitspace(&p);
				emitchar(&p, '<');
				emitchar(&p, c);
				state = ENDTAG;
			} else {
				emitspace(&p);
				emitchar(&p, '<');
				emitchar(&p, c);
				state = STARTTAG;
			}
			break;
		case ENDTAG:
			if (c == '\0') {
				emitword(&p);
				state = END;
			} else if (c == '>') {
				emitchar(&p, c);
				emittag(&p);
				state = START;
			} else {
				emitchar(&p, c);
			}
			break;
		case BEGIN_TAG:
			if (c == '\0') {
				emitspace(&p);
				state = END;
			} else if (c == '/') {
				emitchar(&p, c);
				state = ENDTAG;
			} else {
				emitchar(&p, c);
				state = STARTTAG;
			}
			break;
		case STARTTAG:
			if (c == '\0') {
				emitword(&p);
				state = END;
			} else if (c == '>') {
				emitchar(&p, c);
				emittag(&p);
				state = NEWLINE3;
			} else {
				emitchar(&p, c);
			}
			break;
		case NEWLINE3:
			if (c == '\0') {
                                state = END;
                        } else if (c == '\n') {
				state = START;
			} else if (c == '<') {
				emitchar(&p, c);
				state = BEGIN_TAG;
			} else if (isspace(c)) {
				emitchar(&p, c);
				state = SPACE;
			} else {
				emitchar(&p, c);
				state = WORDPART;
			}
			break;
		case WORDPART:
			if (c == '\0') {
				emitword(&p);
                                state = END;
                        } else if (c == '\n') {
				emitword(&p);
				state = NEWLINE1;
			} else if (isspace(c)) {
				emitword(&p);
				emitchar(&p, c);
				state = SPACE;
			} else if (c == '<') {
				emitword(&p);
				emitchar(&p, c);
				state = BEGIN_TAG;
			} else {
				emitchar(&p, c);
			}
			break;
		default:
			printf("foo\n");
			exit(0);
		}
	}
#else
	fseek(fp, 0, SEEK_END);
	n = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	html = MwMalloc(n+1);
	n = fread(html, 1, n, fp);
	html[n] = '\0';

	r = html;
	while (*r) {
		switch (state) {
		case START:
			if (*r == '\r') {
				r++;
			} else if (!strncmp(r, "<!--", 4)) {
				r += 4;
				state = COMMENT;
			} else if (!strncmp(r, "\n</", 3)) {
				r++;
			} else if (!strncmp(r, "</", 2)) {
				r += 2;
				state = ENDTAG;
			} else if (*r == '<') {
				r++;
				state = STARTTAG;
			} else if (isspace(*r)) {
				emit_space(*r++);
			} else {
				emit_wordpart(*r++);
			}
			break;
		case COMMENT:
			q = strstr(r, "-->");
			if (q) {
				r = q+3;
				state = START;
			} else {
				state = ERROR;
			}
			break;
		case ENDTAG:
			q = strchr(r, '>');
			if (q) {
				*q = '\0';
				emit_endtag(r);
				r = q+1;
				state = START;
			} else {
				state = ERROR;
			}
			break;
		case STARTTAG:
			q = strchr(r, '>');
			if (q) {
				*q = '\0';
				emit_starttag(r);
				r = q+1;
				if (*r == '\r') r++;
				if (*r == '\n') r++;
				state = START;
			} else {
				state = ERROR;
			}
			break;
		case ERROR:
			r += strlen(r);
		}
	}
	flush();
	MwFree(html);
#endif
Reverse:
	add_object_box(&p, MW_HTML_NEWLINE, NULL);

	fclose(fp);

	if (error) {
		fprintf(stderr, "Error\n");
	}
	o1 = MwMalloc(sizeof *o1);
	o1->ui = ui;
	o1->ob = p.ob;
	o1->ref = 1;
	o1->next = oc;
	oc = o1;

	return o1->ob;
}

/* p looks like this: key=value key2 key3="value 2". Keys are not case
   sensitive. Everything not in quotes is converted to upper case.
   Returns value if found, otherwise NULL. Keys without value return "".
*/
char *MwHtmlGetValue(char *k, char *p)
{
        char *q = p;
        int n = strlen(k);
        int i;
        char b[1024];

Again:
        while (*q && isspace(*q)) q++;  /* skip leading space */
        if (!MwStrncasecmp(k, q, n)) {
		if (q[n] == '=') {     /* gotcha */
                	n++;
                	i = 0;
                	if (q[n] == '"') {
                        	n++;
                        	while (q[n] && q[n] != '"') b[i++] = q[n++];
                	} else {
                        	while (q[n] && !isspace(q[n])) b[i++] = q[n++];
                	}
                	b[i] = '\0';
                	return MwStrdup(b);
		} else if (q[n] == '\0' || isspace(q[n])) {
			return MwStrdup("");
		}
        }
        while (*q && !isspace(*q)) q++;
        if (*q) goto Again;
        return NULL;
}

int MwHtmlSave(char *url, char *fn)
{
	url_info *ui = load_url(url);
        FILE *fp1, *fp2;
        int c;
	if (ui == NULL) {
		fprintf(stderr, "Can't get '%s'\n", url);
		return -1;
	}
        fp1 = fopen(ui->local, "r");
        if (fp1 == NULL) return -1;
        fp2 = fopen(fn, "w");
        if (fp2 == NULL) {
		fclose(fp1);
		return -1;
	}
        while ((c = fgetc(fp1)) != EOF) fputc(c, fp2);
	free_url(ui);
        fclose(fp1);
        fclose(fp2);
        return 0;
}
