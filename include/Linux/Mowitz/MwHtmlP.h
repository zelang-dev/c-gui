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

#ifndef s_HtmlP_h
#define s_HtmlP_h

#include "MwHtml.h"
/* include superclass private header file */
#include <X11/ConstrainP.h>
#include <Linux/Mowitz/MwFormat.h>

/* define unique representation types not found in <X11/StringDefs.h> */

#define XtRHtmlResource "HtmlResource"

typedef struct {
    int empty;
} MwHtmlClassPart;

typedef struct s_HtmlClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    MwHtmlClassPart	html_class;
} MwHtmlClassRec;

extern MwHtmlClassRec mwHtmlClassRec;

/* this is almost exactly the same as the rich_text in PW */
typedef struct rich_text {
        int height;             /* in decipoints */
	int indent;		/* in decipoints */
        int sty;		/* no really used */
        int adj;                /* left, right or center */
        char bop;               /* beginning of paragraph */
        MwRichchar *p;
} rich_text;

typedef struct href {
	char *url;		/* url */
	char *name;		/* name */
	object_box *ob1, *ob2;
} href;

typedef struct {
    /* resources */
	char *url;
	int top_row, top_col;
	int total_width, total_height;
	float zoom;
	int delay;
	XtCallbackList callbacks;
	XtCallbackList change_url;
	Widget status;
    /* private state */
	image *fish;
	GC clear_gc;
	GC cell_gc;
	XtIntervalId timeout;
	Cursor cursor;
	object_box *ob;
	href *ref;		/* hyperlinks */
	int nref;		/* number of links */
#ifdef HAVE_XCREATEIC
	XIM xim;
	XIC xic;
#endif
} MwHtmlPart;

typedef struct s_HtmlRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    MwHtmlPart		html;
} MwHtmlRec;

typedef struct MwHtmlConstraintsPart {
	int x, y, w, h;
	object_box *ob;
} MwHtmlConstraintsPart;

typedef struct MwHtmlConstraintsRec {
	MwHtmlConstraintsPart html;
} MwHtmlConstraintsRec, *MwHtmlConstraints;

#endif /* s_HtmlP_h */
