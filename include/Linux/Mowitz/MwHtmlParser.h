#ifndef MW_HTML_PARSER_H
#define MW_HTML_PARSER_H

#include "http.h"

typedef struct MwHtmlNode {
	char *tag;		/* tag name */
	char *text;		/* text */
	struct MwHtmlNode *parent;	/* parent */
	struct MwHtmlNode *next;	/* next sibling */
	struct MwHtmlNode *child;	/* first child */
} MwHtmlNode;

#define MW_HTML_OBJECT 1	/* unused */
#define MW_HTML_BEGIN_A 2	/* <a> tag */
#define MW_HTML_END_A 3		/* </a> tag */
#define MW_HTML_IMAGE 4		/* <img> tag */
#define MW_HTML_HR 5		/* <hr> tag */
#define MW_HTML_WORDPART 6	/* word segment such as foo in foo<i>bar</i> */
#define MW_HTML_NEWLINE 7	/* line break for any reason */
#define MW_HTML_SPACE 8		/* word separator */
#define MW_HTML_INDENT 9	/* indentation level */
#define MW_HTML_UBULLET 10	/* unordered bullet */
#define MW_HTML_OBULLET 11	/* ordered bullet */
#define MW_HTML_BASE 12		/* base url */
#define MW_HTML_TABLE 13	/* <table> */
#define MW_HTML_ROW 15		/* <tr> */
#define MW_HTML_CELL 16		/* <td> or <th> */
#define MW_HTML_FORM 17		/* <form> */
#define MW_HTML_INPUT 18	/* <input ...> */
#define MW_HTML_OPTGROUP 19	/* submenu */
#define MW_HTML_OPTION 20	/* menu/list item */
#define MW_HTML_SELECT 21	/* temporary for select control */
#define MW_HTML_TEXTAREA 22	/* temporary for textarea control */

#define MW_HTML_INPUT_TEXT 1
#define MW_HTML_INPUT_PASSWORD 2
#define MW_HTML_INPUT_CHECKBOX 3
#define MW_HTML_INPUT_RADIO 4
#define MW_HTML_INPUT_SUBMIT 5
#define MW_HTML_INPUT_IMAGE 6
#define MW_HTML_INPUT_RESET 7
#define MW_HTML_INPUT_BUTTON 8
#define MW_HTML_INPUT_HIDDEN 9
#define MW_HTML_INPUT_FILE 10
#define MW_HTML_INPUT_SELECT 11
#define MW_HTML_INPUT_SELECT2 12
#define MW_HTML_INPUT_TEXTAREA 13

typedef struct MwHtmlAnchor {
	char *href;
	char *name;
} MwHtmlAnchor;

typedef struct MwHtmlImage {
	char *src;
	char *alt;
} MwHtmlImage;

typedef struct MwHtmlInput {
	int type;
	struct ui_info *ui;	/* widget and other stuff */
	char *name, *value;
	char *action, *method;
} MwHtmlInput;

typedef struct MwHtmlRow {
	int nchild;
} MwHtmlRow;

typedef struct MwHtmlTable {
	struct {
		int min, max, real;
	} *colwidth;
	int max_child;
} MwHtmlTable;

extern void MwHtmlDump(MwHtmlNode *h, int i);
extern char *MwHtmlGetValue(char *, char *);
extern void MwHtmlFree(object_box *);
extern object_box *MwHtmlParse(char *);
extern int MwHtmlSave(char *, char *fn);

extern void MwHtmlDumpTree(MwHtmlNode *, void *,
		int (*)(void *, int, int),
		void (*)(int, void *, void *));

#endif	/* MW_HTML_PARSER */
