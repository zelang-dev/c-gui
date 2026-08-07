
/* MODIFIED ATHENA SCROLLBAR (USING ARROWHEADS AT ENDS OF TRAVEL) */
/* Modifications Copyright 1992 by Mitch Trachtenberg             */
/* Rights, permissions, and disclaimer of warranty are as in the  */
/* DEC and MIT notice below.                                      */
/* $XConsortium: Scrollbar.c,v 1.72 94/04/17 20:12:40 kaleb Exp $ */

/***********************************************************

Copyright (c) 1987, 1988, 1994  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* ScrollBar.c */
/* created by weissman, Mon Jul  7 13:20:03 1986 */
/* converted by swick, Thu Aug 27 1987 */
/* Modified by Eddie Lau, Sat May 10 1996 */

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <Linux/Xaw95/XawInit.h>
#include <Linux/Xaw95/ScrollbarP.h>
#include <Linux/Xaw95/ThreeD.h>

#include <X11/Xmu/Drawing.h>

int round(float x)
{
    if( x-(float)(int)x >= 0.5 )
	return (int)x+1;
    else
	return (int)x;
}

/* Private definitions. */

#ifdef ARROW_SCROLLBAR
static char defaultTranslations[] =
    "<Btn1Down>:   NotifyScroll()\n\
     <Btn2Down>:   MoveThumb() NotifyThumb() \n\
     <Btn3Down>:   NotifyScroll()\n\
     <Btn1Motion>: HandleThumb() \n\
     <Btn3Motion>: HandleThumb() \n\
     <Btn2Motion>: MoveThumb() NotifyThumb() \n\
     <BtnUp>:      EndScroll()";
#else
static char defaultTranslations[] =
    "<Btn1Down>:   StartScroll(Forward) \n\
     <Btn2Down>:   StartScroll(Continuous) MoveThumb() NotifyThumb() \n\
     <Btn3Down>:   StartScroll(Backward) \n\
     <Btn2Motion>: MoveThumb() NotifyThumb() \n\
     <BtnUp>:      NotifyScroll(Proportional) EndScroll()";
#ifdef bogusScrollKeys
    /* examples */
    "<KeyPress>f:  StartScroll(Forward) NotifyScroll(FullLength) EndScroll()"
    "<KeyPress>b:  StartScroll(Backward) NotifyScroll(FullLength) EndScroll()"
#endif
#endif

static float floatZero = 0.0;

#define Offset(field) XtOffsetOf(ScrollbarRec, field)

static XtResource resources[] = {
#ifdef ARROW_SCROLLBAR
/*  {XtNscrollCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.cursor), XtRString, "crosshair"},*/
#else
  {XtNscrollVCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.verCursor), XtRString, "sb_v_double_arrow"},
  {XtNscrollHCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.horCursor), XtRString, "sb_h_double_arrow"},
  {XtNscrollUCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.upCursor), XtRString, "sb_up_arrow"},
  {XtNscrollDCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.downCursor), XtRString, "sb_down_arrow"},
  {XtNscrollLCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.leftCursor), XtRString, "sb_left_arrow"},
  {XtNscrollRCursor, XtCCursor, XtRCursor, sizeof(Cursor),
       Offset(scrollbar.rightCursor), XtRString, "sb_right_arrow"},
#endif
  {XtNlength, XtCLength, XtRDimension, sizeof(Dimension),
       Offset(scrollbar.length), XtRImmediate, (XtPointer) 1},
  {XtNthickness, XtCThickness, XtRDimension, sizeof(Dimension),
       Offset(scrollbar.thickness), XtRImmediate, (XtPointer) 17},
  {XtNorientation, XtCOrientation, XtROrientation, sizeof(XtOrientation),
      Offset(scrollbar.orientation), XtRImmediate, (XtPointer) XtorientVertical},
  {XtNscrollProc, XtCCallback, XtRCallback, sizeof(XtPointer),
       Offset(scrollbar.scrollProc), XtRCallback, NULL},
  {XtNthumbProc, XtCCallback, XtRCallback, sizeof(XtPointer),
       Offset(scrollbar.thumbProc), XtRCallback, NULL},
  {XtNjumpProc, XtCCallback, XtRCallback, sizeof(XtPointer),
       Offset(scrollbar.jumpProc), XtRCallback, NULL},
  {XtNthumb, XtCThumb, XtRBitmap, sizeof(Pixmap),
       Offset(scrollbar.thumb), XtRImmediate, (XtPointer) XtUnspecifiedPixmap},
  {XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel),
       Offset(scrollbar.foreground), XtRString, /*XtDefaultForeground*/ "gray"},
  {XtNscrollbarBackground, XtCScrollbarBackground, XtRPixel, sizeof(Pixel),
       Offset(scrollbar.background), XtRString, /*XtDefaultForeground*/ "gray"},
  {XtNshown, XtCShown, XtRFloat, sizeof(float),
       Offset(scrollbar.shown), XtRFloat, (XtPointer)&floatZero},
  {XtNtopOfThumb, XtCTopOfThumb, XtRFloat, sizeof(float),
       Offset(scrollbar.top), XtRFloat, (XtPointer)&floatZero},
  {XtNpickTop, XtCPickTop, XtRBoolean, sizeof(Boolean),
       Offset(scrollbar.pick_top), XtRImmediate, (XtPointer) False},
  {XtNminimumThumb, XtCMinimumThumb, XtRDimension, sizeof(Dimension),
       Offset(scrollbar.min_thumb), XtRImmediate, (XtPointer) 7},
  {XtNpushThumb, XtCPushThumb, XtRBoolean, sizeof(Boolean),
       Offset(scrollbar.push_thumb), XtRImmediate, (XtPointer) False},
  {XtNborderWidth, XtCBorderWidth, XtRDimension, sizeof(Dimension),
       Offset(core.border_width), XtRImmediate, 0},
  {XtNshadowWidth, XtCShadowWidth, XtRDimension, sizeof(Dimension),
       Offset(threeD.shadow_width), XtRImmediate, (XtPointer)1}
};
#undef Offset

static void ClassInitialize();
static void Initialize();
static void Destroy();
static void Realize();
static void Resize();
static void Redisplay();
static Boolean SetValues();

#ifdef ARROW_SCROLLBAR
static void HandleThumb();
#else
static void StartScroll();
#endif
static void MoveThumb();
static void NotifyThumb();
static void NotifyScroll();
static void EndScroll();

static XtActionsRec actions[] = {
#ifdef ARROW_SCROLLBAR
    {"HandleThumb",	HandleThumb},
#else
    {"StartScroll",     StartScroll},
#endif
    {"MoveThumb",	MoveThumb},
    {"NotifyThumb",	NotifyThumb},
    {"NotifyScroll",	NotifyScroll},
    {"EndScroll",	EndScroll}
};


ScrollbarClassRec scrollbarClassRec = {
  { /* core fields */
    /* superclass       */	(WidgetClass) &threeDClassRec,
    /* class_name       */	"Scrollbar",
    /* size             */	sizeof(ScrollbarRec),
    /* class_initialize	*/	ClassInitialize,
    /* class_part_init  */	NULL,
    /* class_inited	*/	FALSE,
    /* initialize       */	Initialize,
    /* initialize_hook  */	NULL,
    /* realize          */	Realize,
    /* actions          */	actions,
    /* num_actions	*/	XtNumber(actions),
    /* resources        */	resources,
    /* num_resources    */	XtNumber(resources),
    /* xrm_class        */	NULLQUARK,
    /* compress_motion	*/	TRUE,
    /* compress_exposure*/	TRUE,
    /* compress_enterleave*/	TRUE,
    /* visible_interest */	FALSE,
    /* destroy          */	Destroy,
    /* resize           */	Resize,
    /* expose           */	Redisplay,
    /* set_values       */	SetValues,
    /* set_values_hook  */	NULL,
    /* set_values_almost */	XtInheritSetValuesAlmost,
    /* get_values_hook  */	NULL,
    /* accept_focus     */	NULL,
    /* version          */	XtVersion,
    /* callback_private */	NULL,
    /* tm_table         */	defaultTranslations,
    /* query_geometry	*/	XtInheritQueryGeometry,
    /* display_accelerator*/	XtInheritDisplayAccelerator,
    /* extension        */	NULL
  },
  { /* simple fields */
    /* change_sensitive	*/	XtInheritChangeSensitive
  },
  { /* threeD fields */
    /* shadowdraw	*/	XtInheritXaw3dShadowDraw
  },
  { /* scrollbar fields */
    /* ignore		*/	0
  }

};

WidgetClass scrollbarWidgetClass = (WidgetClass)&scrollbarClassRec;

#define NoButton -1
#define PICKLENGTH(widget, x, y) \
    ((widget->scrollbar.orientation == XtorientHorizontal) ? (x) : (y))
#define MIN(x,y)	((x) < (y) ? (x) : (y))
#define MAX(x,y)	((x) > (y) ? (x) : (y))

static void ClassInitialize()
{
    XawInitializeWidgetSet();
    XtAddConverter( XtRString, XtROrientation, XmuCvtStringToOrientation,
		    (XtConvertArgList)NULL, (Cardinal)0 );
}

#ifdef ARROW_SCROLLBAR
#define MARGIN(sbw) (sbw)->scrollbar.thickness
#else
#define MARGIN(sbw) (sbw)->threeD.shadow_width
#endif

/*
   Used to swap X and Y coordinates when the scrollbar is horizontal.
 */
static void swap(a, b)
    Dimension *a, *b;
{
    Dimension tmp = *a;
    *a = *b;
    *b = tmp;
}

/*
   Paint the thumb in the area specified by sbw->top and sbw->shown.
   The old area is erased.  No special tricks, draws the entire scollbar
   over every time but doesn't draw the same region twice during a
   single call to PaintThumb so there shouldn't be any flicker.
   Draws the thumb "pressed in" when pressed = 1.
 */

static void PaintThumb (sbw, pressed)
    ScrollbarWidget sbw;
    int pressed;
{
    Dimension margin, tzl;
    Position  floor;
    Position  top, bot;
    Dimension x, y;             	   /* upper-left corner of rectangle */
    Dimension w, h;             	   /* size of rectangle */
    Dimension sw = sbw->threeD.shadow_width;
    Dimension th = sbw->scrollbar.thickness;
    XPoint    ipoint[4],opoint[4];	 /* inner and outer points of thumb */
    XPoint    upoint[4];		 /* extra points for 95 style */
    XPoint    point[4];           	 /* points used for drawing */
    GC        top_shadow_GC, bot_shadow_GC;
    GC	      Black_GC;			 /* black border of the objects */
    GC	      utop_shadow_GC;
    GC	      ubot_shadow_GC;
    double    thumb_len;
    static char pattern[]={0xaa,0x55};	 /* Fill pattern Win95 style */
    Pixmap    map;

    margin = MARGIN (sbw);
    tzl = sbw->scrollbar.length - 2*margin;
    floor = sbw->scrollbar.length - margin;

    top = margin + (int)(tzl * sbw->scrollbar.top);
    thumb_len = tzl * sbw->scrollbar.shown;
    bot = top + (int)thumb_len;
    if ( thumb_len-(int)thumb_len > 0.5 ) ++bot;

    if (bot < top + (int)sbw->scrollbar.min_thumb +
              2 * (int)sbw->threeD.shadow_width)
	bot = top + sbw->scrollbar.min_thumb +
	      2 * sbw->threeD.shadow_width;

    if ( bot >= floor ) {
	top = floor-(bot-top)+1;
	bot = floor;
    }

    sbw->scrollbar.topLoc = top;
    sbw->scrollbar.shownLength = bot - top;

    if ( XtIsRealized((Widget)sbw) )
    {
	Black_GC=XCreateGC( XtDisplay((Widget)sbw), XtWindow((Widget)sbw), 0, 0);
	XSetLineAttributes( XtDisplay((Widget)sbw), Black_GC, 1, LineSolid,
			    CapNotLast, JoinMiter );
	XSetForeground( XtDisplay((Widget)sbw), Black_GC,
		    	    XBlackPixel( XtDisplay((Widget)sbw),
					 DefaultScreen(XtDisplay((Widget)sbw))) );

	if (pressed && sbw->scrollbar.push_thumb) {
	    top_shadow_GC = sbw->threeD.bot_shadow_GC;
	    bot_shadow_GC = sbw->threeD.top_shadow_GC;
	    utop_shadow_GC = Black_GC;
	    ubot_shadow_GC = sbw->scrollbar.bgc;
        }
	else {
	    top_shadow_GC = sbw->threeD.top_shadow_GC;
	    bot_shadow_GC = sbw->threeD.bot_shadow_GC;
	    utop_shadow_GC = sbw->scrollbar.bgc;
	    ubot_shadow_GC = Black_GC;
	}

	/* the space above the thumb */
	x = 0;
	y = margin;
	w = th;
	h = top - y;
	if (sbw->scrollbar.orientation == XtorientHorizontal) {
	    swap(&x, &y);
	    swap(&w, &h);
	}
	XSetBackground( XtDisplay((Widget)sbw), sbw->scrollbar.bgc,
			XWhitePixel( XtDisplay((Widget)sbw),
				     DefaultScreen(XtDisplay((Widget)sbw))) );
	map=XCreateBitmapFromData(XtDisplay((Widget)sbw), XtWindow((Widget)sbw),
				  pattern, 8, 2);
	XSetStipple(XtDisplay((Widget)sbw), sbw->scrollbar.bgc, map);
	XSetFillStyle(XtDisplay((Widget)sbw), sbw->scrollbar.bgc,
		      FillOpaqueStippled);
	XFillRectangle(XtDisplay((Widget) sbw), XtWindow((Widget) sbw),
		       sbw->scrollbar.bgc, x, y,
		       (unsigned int) w, (unsigned int) h);
	XSetFillStyle(XtDisplay((Widget)sbw), sbw->scrollbar.bgc, FillSolid);
	/* the space below the thumb */
	x = 0;
	y = bot;
	w = th;
	h = tzl + margin - bot;
	if (sbw->scrollbar.orientation == XtorientHorizontal) {
	    swap(&x, &y);
	    swap(&w, &h);
	}
	XSetFillStyle(XtDisplay((Widget)sbw), sbw->scrollbar.bgc,
		      FillOpaqueStippled);
	XFillRectangle(XtDisplay((Widget) sbw), XtWindow((Widget) sbw),
		       sbw->scrollbar.bgc, x, y,
		       (unsigned int) w, (unsigned int) h);
	XSetFillStyle(XtDisplay((Widget)sbw), sbw->scrollbar.bgc, FillSolid);
	/* the thumb itself */
	x = sw * 2;
	y = top + sw*2;
	w = th - sw * 4;
	h = bot - top - 2 * sw;
	if (sbw->scrollbar.orientation == XtorientHorizontal) {
	    swap(&x, &y);
	    swap(&w, &h);
	}
	/* we can't use "w > 0" and "h > 0" because they are
	   usually unsigned quantities */

	if (th - sw * 4 > 0 && bot - top - 2 * sw > 0)
	    XFillRectangle(XtDisplay((Widget) sbw), XtWindow((Widget) sbw),
			   sbw->scrollbar.bgc, x, y,
			   (unsigned int) w, (unsigned int) h);

	/* the shades around the thumb

        e0                   e3
         +------------------+
	 |\ o0          o3 /|
	 | +--------------+ |
	 | |\ i0      i3 /| |
	 | | +----------+ | |
	 | | |          | | |
	 | | |          | | |
	 | | |          | | |
	 | | +----------+ | |
	 | |/ i1      i2 \| |
	 | +--------------+ |
	 |/ o1          o2 \|
         +------------------+
        e1                  e2
	 */

	upoint[0].x = upoint[1].x = 0;
	upoint[0].y = upoint[3].y = top;
	upoint[2].x = upoint[3].x = th;
	upoint[2].y = upoint[1].y = bot;

	opoint[0].x = opoint[1].x = upoint[0].x+sw;
	opoint[0].y = opoint[3].y = upoint[0].y+sw;
	opoint[2].x = opoint[3].x = upoint[2].x-sw;
	opoint[2].y = opoint[1].y = upoint[2].y-sw;;

	ipoint[0].x = ipoint[1].x = opoint[0].x + sw;
	ipoint[0].y = ipoint[3].y = opoint[0].y + sw;
	ipoint[2].x = ipoint[3].x = opoint[2].x - sw;
	ipoint[2].y = ipoint[1].y = opoint[2].y - sw;

	/* make sure shades don't overlap */
	if (ipoint[0].x > ipoint[3].x)
	    ipoint[3].x = ipoint[2].x = ipoint[1].x = ipoint[0].x
		= (ipoint[0].x + ipoint[3].x) / 2;
	if (ipoint[0].y > ipoint[1].y)
	    ipoint[3].y = ipoint[2].y = ipoint[1].y = ipoint[0].y
		= (ipoint[0].y + ipoint[1].y) / 2;
	if (sbw->scrollbar.orientation == XtorientHorizontal) {
	    int n;
	    for (n = 0; n < 4; n++)	{
		swap(&ipoint[n].x, &ipoint[n].y);
		swap(&opoint[n].x, &opoint[n].y);
		swap(&upoint[n].x, &upoint[n].y);
	    }
	}

	XDrawLine( XtDisplay((Widget)sbw),
		   XtWindow((Widget)sbw),
		   Black_GC, opoint[3].x, opoint[3].y,
		   opoint[2].x, opoint[2].y );
	XDrawLine( XtDisplay( (Widget)sbw ),
		   XtWindow((Widget)sbw),
		   Black_GC, opoint[1].x, opoint[1].y,
		   opoint[2].x, opoint[2].y );

	/* left */
	point[0] = opoint[0];
	point[1] = opoint[1];
	point[2] = ipoint[1];
	point[3] = ipoint[0];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      top_shadow_GC, point, 4, Convex, CoordModeOrigin);

	point[0] = upoint[0];
	point[1] = upoint[1];
	point[2] = opoint[1];
	point[3] = opoint[0];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      utop_shadow_GC, point, 4, Convex, CoordModeOrigin);

	/* top */
	point[0] = opoint[0];
	point[1] = opoint[3];
	point[2] = ipoint[3];
	point[3] = ipoint[0];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      top_shadow_GC, point, 4, Convex, CoordModeOrigin);
	point[0] = upoint[0];
	point[1] = upoint[3];
	point[2] = opoint[3];
	point[3] = opoint[0];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      utop_shadow_GC, point, 4, Convex, CoordModeOrigin);
	/* bottom */
	point[0] = opoint[1];
	point[1] = opoint[2];
	point[2] = ipoint[2];
	point[3] = ipoint[1];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      bot_shadow_GC, point, 4, Convex, CoordModeOrigin);
	point[0] = upoint[1];
	point[1] = upoint[2];
	point[2] = opoint[2];
	point[3] = opoint[1];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      ubot_shadow_GC, point, 4, Convex, CoordModeOrigin);
	/* right */
	point[0] = opoint[3];
	point[1] = opoint[2];
	point[2] = ipoint[2];
	point[3] = ipoint[3];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      bot_shadow_GC, point, 4, Convex, CoordModeOrigin);
	point[0] = upoint[3];
	point[1] = upoint[2];
	point[2] = opoint[2];
	point[3] = opoint[3];
	XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		      ubot_shadow_GC, point, 4, Convex, CoordModeOrigin);

	XFreeGC(XtDisplay((Widget)sbw),Black_GC);
    }
}

#ifdef ARROW_SCROLLBAR
/*
   Draw the arrows for scrollbar.
   Draw the corresponding arrow "pressed in" if
   toppressed or botpressed is 1.
 */
static void PaintArrows (sbw, toppressed, botpressed)
    ScrollbarWidget sbw;
    int toppressed, botpressed;
{
    int		n;			/* 2 arrows */
    int		pressed;		/* Is pressed? */
    XPoint	ipoint[4], opoint[4];	/* inner and outer points */
    XPoint	upoint[4];		/* the rectangle */
    XPoint	point[4];           	 /* points used for drawing */
    XPoint	apoint[3];		/* points used for arrows */
    Dimension	x,y,w,h,top;
    Dimension	sw = sbw->threeD.shadow_width;
    Dimension	th = sbw->scrollbar.thickness;
    Dimension	len = sbw->scrollbar.length;
    GC		top_shadow_GC, bot_shadow_GC;
    GC		Black_GC;		/* black border of the objects */
    GC		utop_shadow_GC;
    GC		ubot_shadow_GC;

    if (XtIsRealized ((Widget) sbw)) {
	if (sw) {
	  pressed=toppressed;
	  top=0;
	  for(n=0;n<=2;n++)
          {
	    Black_GC=XCreateGC( XtDisplay((Widget)sbw), XtWindow((Widget)sbw), 0, 0);
	    XSetLineAttributes( XtDisplay((Widget)sbw), Black_GC, 1, LineSolid,
			        CapNotLast, JoinMiter );
	    XSetForeground( XtDisplay((Widget)sbw), Black_GC,
			    XBlackPixel( XtDisplay((Widget)sbw),
				         DefaultScreen(XtDisplay((Widget)sbw))) );
	    if (pressed) {
		top_shadow_GC = sbw->threeD.bot_shadow_GC;
		bot_shadow_GC = sbw->threeD.top_shadow_GC;
		utop_shadow_GC = Black_GC;
		ubot_shadow_GC = sbw->scrollbar.bgc;
	    }
	    else {
		top_shadow_GC = sbw->threeD.top_shadow_GC;
		bot_shadow_GC = sbw->threeD.bot_shadow_GC;
		utop_shadow_GC = sbw->scrollbar.bgc;
		ubot_shadow_GC = Black_GC;
	    }

	    x = sw * 2;
	    y = sw * 2 + top;
	    w = th - sw * 4;
	    h = th - 2 * sw;
	    if (sbw->scrollbar.orientation == XtorientHorizontal) {
		swap(&x, &y);
		swap(&w, &h);
	    }
	    /* we can't use "w > 0" and "h > 0" because they are
		usually unsigned quantities */

	    if (th - sw * 4 > 0)
		XFillRectangle(XtDisplay((Widget) sbw), XtWindow((Widget) sbw),
				sbw->scrollbar.bgc, x, y,
				(unsigned int) w, (unsigned int) h);

	        /* the shades around the thumb

            e0                   e3
             +------------------+
             |\ o0          o3 /|
	     | +--------------+ |
	     | |\ i0      i3 /| |
	     | | +----------+ | |
	     | | |    /\ a0 | | |
	     | | |   /  \   | | |
	     | | |a1+----+a2| | |
	     | | +----------+ | |
	     | |/ i1      i2 \| |
	     | +--------------+ |
	     |/ o1          o2 \|
             +------------------+
            e1                  e2
	     */

	    upoint[0].x = upoint[1].x = 0;
	    upoint[0].y = upoint[3].y = 0 + top;
	    upoint[2].x = upoint[3].x = th;
	    upoint[2].y = upoint[1].y = th + top;

	    opoint[0].x = opoint[1].x = upoint[0].x+sw;
	    opoint[0].y = opoint[3].y = upoint[0].y+sw;
	    opoint[2].x = opoint[3].x = upoint[2].x-sw;
	    opoint[2].y = opoint[1].y = upoint[2].y-sw;

	    ipoint[0].x = ipoint[1].x = opoint[0].x + sw;
	    ipoint[0].y = ipoint[3].y = opoint[0].y + sw;
	    ipoint[2].x = ipoint[3].x = opoint[2].x - sw;
	    ipoint[2].y = ipoint[1].y = opoint[2].y - sw;

	    if(top==0)
	    {
		apoint[0].x = 0.5*(ipoint[0].x+ipoint[3].x);
		apoint[0].y = ipoint[0].y+0.25*(ipoint[1].y-ipoint[0].y);
		apoint[1].x = ipoint[0].x+0.25*(ipoint[3].x-ipoint[0].x);
		apoint[1].y = ipoint[1].y-0.5*(ipoint[1].y-ipoint[0].y);
		apoint[2].x = ipoint[3].x-0.25*(ipoint[3].x-ipoint[0].x);
		apoint[2].y = apoint[1].y;
	    }
	    else
	    {
		apoint[0].x = 0.5*(ipoint[0].x+ipoint[3].x);
		apoint[0].y = ipoint[1].y-0.25*(ipoint[1].y-ipoint[0].y);
		apoint[2].x = ipoint[0].x+0.25*(ipoint[3].x-ipoint[0].x);
		apoint[2].y = ipoint[0].y+0.5*(ipoint[1].y-ipoint[0].y);
		apoint[1].x = ipoint[3].x-0.25*(ipoint[3].x-ipoint[0].x);
		apoint[1].y = apoint[2].y;
	    }
	    if(pressed)
	    {
		apoint[0].x+=1;
		apoint[0].y+=1;
		apoint[1].x+=1;
		apoint[1].y+=1;
		apoint[2].x+=1;
		apoint[2].y+=1;
	    }

	    /* make sure shades don't overlap */
	    if (ipoint[0].x > ipoint[3].x)
	        ipoint[3].x = ipoint[2].x = ipoint[1].x = ipoint[0].x
		    = (ipoint[0].x + ipoint[3].x) / 2;
	    if (ipoint[0].y > ipoint[1].y)
	        ipoint[3].y = ipoint[2].y = ipoint[1].y = ipoint[0].y
		    = (ipoint[0].y + ipoint[1].y) / 2;
	    if (sbw->scrollbar.orientation == XtorientHorizontal) {
	        int n;
	        for (n = 0; n < 4; n++)	{
		    swap(&ipoint[n].x, &ipoint[n].y);
		    swap(&opoint[n].x, &opoint[n].y);
		    swap(&upoint[n].x, &upoint[n].y);
		    if(n<3)
			swap(&apoint[n].x, &apoint[n].y);
	        }
	    }
	    else
	    {
		if(top==0)
		{
		    apoint[1].x-=1;	/* Adjustment of the arrow */
		    apoint[2].x+=1;
		    apoint[1].y+=1;
		    apoint[2].y+=1;
		}
	    }

	    XDrawLine( XtDisplay((Widget)sbw),
		       XtWindow((Widget)sbw),
		       Black_GC, opoint[3].x, opoint[3].y,
		       opoint[2].x, opoint[2].y );
	    XDrawLine( XtDisplay( (Widget)sbw ),
		       XtWindow((Widget)sbw),
		       Black_GC, opoint[1].x, opoint[1].y,
		       opoint[2].x, opoint[2].y );

	    /* left */
	    point[0] = opoint[0];
	    point[1] = opoint[1];
	    point[2] = ipoint[1];
	    point[3] = ipoint[0];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          top_shadow_GC, point, 4, Convex, CoordModeOrigin);

	    point[0] = upoint[0];
	    point[1] = upoint[1];
	    point[2] = opoint[1];
	    point[3] = opoint[0];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          utop_shadow_GC, point, 4, Convex, CoordModeOrigin);

	    /* top */
	    point[0] = opoint[0];
	    point[1] = opoint[3];
	    point[2] = ipoint[3];
	    point[3] = ipoint[0];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          top_shadow_GC, point, 4, Convex, CoordModeOrigin);
	    point[0] = upoint[0];
	    point[1] = upoint[3];
	    point[2] = opoint[3];
	    point[3] = opoint[0];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          utop_shadow_GC, point, 4, Convex, CoordModeOrigin);
	    /* bottom */
	    point[0] = opoint[1];
	    point[1] = opoint[2];
	    point[2] = ipoint[2];
	    point[3] = ipoint[1];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          bot_shadow_GC, point, 4, Convex, CoordModeOrigin);
	    point[0] = upoint[1];
	    point[1] = upoint[2];
	    point[2] = opoint[2];
	    point[3] = opoint[1];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          ubot_shadow_GC, point, 4, Convex, CoordModeOrigin);
	    /* right */
	    point[0] = opoint[3];
	    point[1] = opoint[2];
	    point[2] = ipoint[2];
	    point[3] = ipoint[3];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          bot_shadow_GC, point, 4, Convex, CoordModeOrigin);
	    point[0] = upoint[3];
	    point[1] = upoint[2];
	    point[2] = opoint[2];
	    point[3] = opoint[3];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          ubot_shadow_GC, point, 4, Convex, CoordModeOrigin);

	    point[0] = apoint[0];
	    point[1] = apoint[1];
	    point[2] = apoint[2];
	    XFillPolygon (XtDisplay ((Widget) sbw), XtWindow ((Widget) sbw),
		          Black_GC, point, 3, Convex, CoordModeOrigin);

	    pressed=botpressed;
	    top=len-th;
	  }
        }
    }
}
#endif

/*	Function Name: Destroy
 *	Description: Called as the scrollbar is going away...
 *	Arguments: w - the scrollbar.
 *	Returns: nonw
 */
static void Destroy (w)
    Widget w;
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
#ifdef ARROW_SCROLLBAR
    if(sbw->scrollbar.timer_id != (XtIntervalId) 0)
	XtRemoveTimeOut (sbw->scrollbar.timer_id);
#endif
    XtReleaseGC (w, sbw->scrollbar.gc);
    XtReleaseGC (w, sbw->scrollbar.bgc);
}

/*	Function Name: CreateGC
 *	Description: Creates the GC.
 *	Arguments: w - the scrollbar widget.
 *	Returns: none.
 */

static void CreateGC (w)
    Widget w;
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    XGCValues gcValues;
    XtGCMask mask;
    XColor shadow;
    unsigned int depth = 1;
    Display *dpy = XtDisplay (sbw);
    Screen *scn = XtScreen (sbw);
    /*    Colormap cmap = DefaultColormapOfScreen (scn);*/
    Colormap cmap = DefaultColormap(dpy,XScreenNumberOfScreen(scn));

    /* make GC for scrollbar background */
    if (sbw->threeD.be_nice_to_cmap ||
	DefaultDepthOfScreen (XtScreen(w)) == 1) {
	mask = GCTile | GCFillStyle;
	gcValues.tile = sbw->threeD.bot_shadow_pxmap;
	gcValues.fill_style = FillTiled;
    } else {
	mask = GCForeground;
	gcValues.foreground = sbw->scrollbar.background;
    }
    sbw->scrollbar.bgc = XtGetGC(w, mask, &gcValues);

    /* make GC for scrollbar foreground */
    if (sbw->scrollbar.thumb == XtUnspecifiedPixmap) {
        sbw->scrollbar.thumb = XmuCreateStippledPixmap (XtScreen(w),
					(Pixel) 0, (Pixel) 0, depth);
    } else if (sbw->scrollbar.thumb != None) {
	Window root;
	int x, y;
	unsigned int width, height, bw;
	if (XGetGeometry (XtDisplay(w), sbw->scrollbar.thumb, &root, &x, &y,
			 &width, &height, &bw, &depth) == 0) {
	    XtAppError (XtWidgetToApplicationContext (w),
	       "Scrollbar Widget: Could not get geometry of thumb pixmap.");
	}
    }

    gcValues.foreground = sbw->scrollbar.foreground;
    gcValues.background = sbw->core.background_pixel;
    mask = GCForeground | GCBackground;

    if (sbw->scrollbar.thumb != None) {
	if (depth == 1) {
	    gcValues.fill_style = FillOpaqueStippled;
	    gcValues.stipple = sbw->scrollbar.thumb;
	    mask |= GCFillStyle | GCStipple;
	}
	else {
	    gcValues.fill_style = FillTiled;
	    gcValues.tile = sbw->scrollbar.thumb;
	    mask |= GCFillStyle | GCTile;
	}
    }
    /* the creation should be non-caching, because */
    /* we now set and clear clip masks on the gc returned */
/*    gcValues.foreground = sbw->scrollbar.foreground;*/
    gcValues.foreground = sbw->core.background_pixel;
    gcValues.background = sbw->core.background_pixel;
    mask = GCForeground | GCBackground;
    sbw->scrollbar.gc = XtGetGC (w, mask, &gcValues);

    sbw->core.background_pixel=sbw->scrollbar.background;
    Xaw3dComputeBottomShadowRGB(w, &shadow);
    XAllocColor(dpy, cmap, &shadow);
    gcValues.foreground = shadow.pixel;
    gcValues.background = sbw->core.background_pixel;
    mask = GCForeground | GCBackground;
    sbw->threeD.bot_shadow_GC=XtGetGC(w, mask, &gcValues);

    Xaw3dComputeTopShadowRGB(w, &(shadow));
    XAllocColor(dpy, cmap, &shadow);
    gcValues.foreground = shadow.pixel;
    gcValues.background = sbw->core.background_pixel;
    mask = GCForeground | GCBackground;
    sbw->threeD.top_shadow_GC=XtGetGC(w, mask, &gcValues);

}

static void SetDimensions (sbw)
    ScrollbarWidget sbw;
{
    if (sbw->scrollbar.orientation == XtorientVertical) {
	sbw->scrollbar.length = sbw->core.height;
	sbw->scrollbar.thickness = sbw->core.width;
    } else {
	sbw->scrollbar.length = sbw->core.width;
	sbw->scrollbar.thickness = sbw->core.height;
    }
}

/* ARGSUSED */
static void Initialize( request, new, args, num_args )
    Widget request;		/* what the client asked for */
    Widget new;			/* what we're going to give him */
    ArgList args;
    Cardinal *num_args;
{
    ScrollbarWidget sbw = (ScrollbarWidget) new;

    CreateGC (new);

    if (sbw->core.width == 0)
	sbw->core.width = (sbw->scrollbar.orientation == XtorientVertical)
	    ? sbw->scrollbar.thickness : sbw->scrollbar.length;

    if (sbw->core.height == 0)
	sbw->core.height = (sbw->scrollbar.orientation == XtorientHorizontal)
	    ? sbw->scrollbar.thickness : sbw->scrollbar.length;

    SetDimensions (sbw);
#ifdef ARROW_SCROLLBAR
    sbw->scrollbar.scroll_mode = 0;
    sbw->scrollbar.timer_id = (XtIntervalId)0;
#endif
    sbw->scrollbar.topLoc = 0;
    sbw->scrollbar.shownLength = sbw->scrollbar.min_thumb;
}

static void Realize (w, valueMask, attributes)
    Widget w;
    Mask *valueMask;
    XSetWindowAttributes *attributes;
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
#ifdef ARROW_SCROLLBAR
    if(sbw->simple.cursor_name == NULL)
	XtVaSetValues(w, XtNcursorName, "crosshair", NULL);
    /* dont set the cursor of the window to anything */
    *valueMask &= ~CWCursor;
#else
    sbw->scrollbar.inactiveCursor =
      (sbw->scrollbar.orientation == XtorientVertical)
	? sbw->scrollbar.verCursor
	: sbw->scrollbar.horCursor;

    XtVaSetValues (w, XtNcursor, sbw->scrollbar.inactiveCursor, NULL);
#endif
    /*
     * The Simple widget actually stuffs the value in the valuemask.
     */

    (*scrollbarWidgetClass->core_class.superclass->core_class.realize)
	(w, valueMask, attributes);
}

/* ARGSUSED */
static Boolean SetValues (current, request, desired, args, num_args)
    Widget  current,		/* what I am */
	    request,		/* what he wants me to be */
	    desired;		/* what I will become */
    ArgList args;
    Cardinal *num_args;
{
    ScrollbarWidget sbw = (ScrollbarWidget) current;
    ScrollbarWidget dsbw = (ScrollbarWidget) desired;
    Boolean redraw = FALSE;

/*
 * If these values are outside the acceptable range ignore them...
 */

    if (dsbw->scrollbar.top < 0.0 || dsbw->scrollbar.top > 1.0)
        dsbw->scrollbar.top = sbw->scrollbar.top;

    if (dsbw->scrollbar.shown < 0.0 || dsbw->scrollbar.shown > 1.0)
        dsbw->scrollbar.shown = sbw->scrollbar.shown;

/*
 * Change colors and stuff...
 */
    if (XtIsRealized (desired)) {
	if (sbw->scrollbar.foreground != dsbw->scrollbar.foreground ||
	    sbw->core.background_pixel != dsbw->core.background_pixel ||
	    sbw->scrollbar.background != dsbw->scrollbar.background) {
	    XtReleaseGC (desired, sbw->scrollbar.gc);
	    XtReleaseGC (desired, sbw->scrollbar.bgc);
	    CreateGC (desired);
	    redraw = TRUE;
	}
	if (sbw->scrollbar.top != dsbw->scrollbar.top ||
	    sbw->scrollbar.shown != dsbw->scrollbar.shown)
	    redraw = TRUE;
    }
    return redraw;
}

static void Resize (w)
    Widget w;
{
    /* ForgetGravity has taken care of background, but thumb may
     * have to move as a result of the new size. */
    SetDimensions ((ScrollbarWidget) w);
    Redisplay (w, (XEvent*) NULL, (Region)NULL);
}


/* ARGSUSED */
static void Redisplay (w, event, region)
    Widget w;
    XEvent *event;
    Region region;
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    int x, y;
    unsigned int width, height;

/*    (*swclass->threeD_class.shadowdraw) (w, event, region, FALSE); */
    x = y = 1;
    width = sbw->core.width - 2;
    height = sbw->core.height - 2;
    if (region == NULL ||
	XRectInRegion (region, x, y, width, height) != RectangleOut) {
	/* Forces entire thumb to be painted. */
	sbw->scrollbar.topLoc = -(sbw->scrollbar.length + 1);
	PaintThumb (sbw, 0);
    }
#ifdef ARROW_SCROLLBAR
    /* we'd like to be region aware here!!!! */
    PaintArrows (sbw, 0, 0);
#endif
}

static Boolean CompareEvents (oldEvent, newEvent)
    XEvent *oldEvent, *newEvent;
{
#define Check(field) if (newEvent->field != oldEvent->field) return False;

    Check(xany.display);
    Check(xany.type);
    Check(xany.window);

    switch (newEvent->type) {
    case MotionNotify:
	Check(xmotion.state);
	break;
    case ButtonPress:
    case ButtonRelease:
	Check(xbutton.state);
	Check(xbutton.button);
	break;
    case KeyPress:
    case KeyRelease:
	Check(xkey.state);
	Check(xkey.keycode);
	break;
    case EnterNotify:
    case LeaveNotify:
	Check(xcrossing.mode);
	Check(xcrossing.detail);
	Check(xcrossing.state);
	break;
    }
#undef Check

    return True;
}

struct EventData {
    XEvent *oldEvent;
    int count;
};

static Bool PeekNotifyEvent (dpy, event, args)
    Display *dpy;
    XEvent *event;
    char *args;
{
    struct EventData *eventData = (struct EventData*)args;

    return ((++eventData->count == QLength(dpy)) /* since PeekIf blocks */
	    || CompareEvents(event, eventData->oldEvent));
}


static Boolean LookAhead (w, event)
    Widget w;
    XEvent *event;
{
    XEvent newEvent;
    struct EventData eventData;

    if (QLength (XtDisplay (w)) == 0) return False;

    eventData.count = 0;
    eventData.oldEvent = event;

    XPeekIfEvent (XtDisplay (w), &newEvent, PeekNotifyEvent, (char*)&eventData);

    return CompareEvents (event, &newEvent);
}


static void ExtractPosition (event, x, y)
    XEvent *event;
    Position *x, *y;		/* RETURN */
{
    switch( event->type ) {
    case MotionNotify:
	*x = event->xmotion.x;
	*y = event->xmotion.y;
	break;
    case ButtonPress:
    case ButtonRelease:
	*x = event->xbutton.x;
	*y = event->xbutton.y;
	break;
    case KeyPress:
    case KeyRelease:
	*x = event->xkey.x;
	*y = event->xkey.y;
	break;
    case EnterNotify:
    case LeaveNotify:
	*x = event->xcrossing.x;
	*y = event->xcrossing.y;
	break;
    default:
	*x = 0; *y = 0;
    }
}

#ifdef ARROW_SCROLLBAR
/* ARGSUSED */
static void HandleThumb (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
    Position x,y;
    ScrollbarWidget sbw = (ScrollbarWidget) w;

    ExtractPosition( event, &x, &y );
    /* if the motion event puts the pointer in thumb, call Move and Notify */
    /* also call Move and Notify if we're already in continuous scroll mode */
    if (sbw->scrollbar.scroll_mode == 2 ||
	(PICKLENGTH (sbw,x,y) >= sbw->scrollbar.topLoc &&
	 PICKLENGTH (sbw,x,y) <= (sbw->scrollbar.topLoc +
				  sbw->scrollbar.shownLength))){
	XtCallActionProc(w, "MoveThumb", event, params, *num_params);
	XtCallActionProc(w, "NotifyThumb", event, params, *num_params);
    }
}

static void RepeatNotify (client_data, idp)
    XtPointer client_data;
    XtIntervalId *idp;
{
#define A_FEW_PIXELS 5
    ScrollbarWidget sbw = (ScrollbarWidget) client_data;
    int call_data;

    if (sbw->scrollbar.scroll_mode != 1 && sbw->scrollbar.scroll_mode != 3) {
	sbw->scrollbar.timer_id = (XtIntervalId) 0;
	return;
    }
    call_data = MAX (A_FEW_PIXELS, sbw->scrollbar.length / 20);
    if (sbw->scrollbar.scroll_mode == 1)
	call_data = -call_data;
    XtCallCallbacks((Widget)sbw, XtNscrollProc, (XtPointer) call_data);
    sbw->scrollbar.timer_id =
    XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)sbw),
		    (unsigned long) 150,
		    RepeatNotify,
		    client_data);
}

#else /* ARROW_SCROLLBAR */
/* ARGSUSED */
static void StartScroll (w, event, params, num_params )
    Widget w;
    XEvent *event;
    String *params;		/* direction: Back|Forward|Smooth */
    Cardinal *num_params;	/* we only support 1 */
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    Cursor cursor;
    char direction;

    if (sbw->scrollbar.direction != 0) return; /* if we're already scrolling */
    if (*num_params > 0)
	direction = *params[0];
    else
	direction = 'C';

    sbw->scrollbar.direction = direction;

    switch (direction) {
    case 'B':
    case 'b':
	cursor = (sbw->scrollbar.orientation == XtorientVertical)
			? sbw->scrollbar.downCursor
			: sbw->scrollbar.rightCursor;
	break;
    case 'F':
    case 'f':
	cursor = (sbw->scrollbar.orientation == XtorientVertical)
			? sbw->scrollbar.upCursor
			: sbw->scrollbar.leftCursor;
	break;
    case 'C':
    case 'c':
	cursor = (sbw->scrollbar.orientation == XtorientVertical)
			? sbw->scrollbar.rightCursor
			: sbw->scrollbar.upCursor;
	break;
    default:
	return; /* invalid invocation */
    }
    XtVaSetValues (w, XtNcursor, cursor, NULL);
    XFlush (XtDisplay (w));
}
#endif /* ARROW_SCROLLBAR */

/*
 * Make sure the first number is within the range specified by the other
 * two numbers.
 */

#ifndef ARROW_SCROLLBAR
static int InRange(num, small, big)
    int num, small, big;
{
    return (num < small) ? small : ((num > big) ? big : num);
}
#endif

/*
 * Same as above, but for floating numbers.
 */

static float FloatInRange(num, small, big)
    float num, small, big;
{
    return (num < small) ? small : ((num > big) ? big : num);
}


#ifdef ARROW_SCROLLBAR
static void NotifyScroll (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    int call_data;
    Position x, y;

    if (sbw->scrollbar.scroll_mode == 2) return; /* if scroll continuous */

    if (LookAhead (w, event)) return;

    ExtractPosition (event, &x, &y);

    if (PICKLENGTH (sbw,x,y) < sbw->scrollbar.thickness) {
	/* handle first arrow zone */
	call_data = MAX (A_FEW_PIXELS, sbw->scrollbar.length / 20);
	XtCallCallbacks (w, XtNscrollProc, (XtPointer)(-call_data));
	/* establish autoscroll */
	sbw->scrollbar.timer_id =
	    XtAppAddTimeOut (XtWidgetToApplicationContext (w),
			     (unsigned long) 300, RepeatNotify, (XtPointer)w);
	sbw->scrollbar.scroll_mode = 1;
	PaintArrows (sbw, 1, 0);
	return;
    } else if (PICKLENGTH (sbw,x,y) > sbw->scrollbar.length -
				      sbw->scrollbar.thickness) {
	/* handle last arrow zone */
	call_data = MAX (A_FEW_PIXELS, sbw->scrollbar.length / 20);
	XtCallCallbacks (w, XtNscrollProc, (XtPointer)(call_data));
	/* establish autoscroll */
	sbw->scrollbar.timer_id =
	    XtAppAddTimeOut (XtWidgetToApplicationContext (w),
			     (unsigned long) 300, RepeatNotify, (XtPointer)w);
	sbw->scrollbar.scroll_mode = 3;
	PaintArrows (sbw, 0, 1);
	return;
    } else if (PICKLENGTH (sbw, x, y) < sbw->scrollbar.topLoc) {
	/* handle zone "above" the thumb */
	call_data = -(sbw->scrollbar.length);
	XtCallCallbacks (w, XtNscrollProc, (XtPointer)(call_data));
	return;
    } else if (PICKLENGTH (sbw, x, y) > sbw->scrollbar.topLoc +
					sbw->scrollbar.shownLength) {
	/* handle zone "below" the thumb */
	call_data = sbw->scrollbar.length;
	XtCallCallbacks (w, XtNscrollProc, (XtPointer)(call_data));
	return;
    } else {
	/* handle the thumb in the motion notify action */
	/* but we need to re-paint it "pressed in" here */
	PaintThumb (sbw, 1);
	return;
    }
}
#else /* ARROW_SCROLLBAR */
static void NotifyScroll (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;		/* style: Proportional|FullLength */
    Cardinal *num_params;	/* we only support 1 */
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    int call_data;
    char style;
    Position x, y;

    if (sbw->scrollbar.direction == 0) return; /* if no StartScroll */
    if (LookAhead (w, event)) return;
    if (*num_params > 0)
	style = *params[0];
    else
	style = 'P';

    switch (style) {
    case 'P':    /* Proportional */
    case 'p':
	ExtractPosition (event, &x, &y);
	call_data =
	    InRange (PICKLENGTH (sbw, x, y), 0, (int) sbw->scrollbar.length);
	break;

    case 'F':    /* FullLength */
    case 'f':
	call_data = sbw->scrollbar.length;
	break;
    }
    switch (sbw->scrollbar.direction) {
    case 'B':
    case 'b':
	call_data = -call_data;
	/* fall through */

    case 'F':
    case 'f':
	XtCallCallbacks (w, XtNscrollProc, (XtPointer)call_data);
	break;

    case 'C':
    case 'c':
	/* NotifyThumb has already called the thumbProc(s) */
	break;
    }
}
#endif /* ARROW_SCROLLBAR */

/* ARGSUSED */
static void EndScroll(w, event, params, num_params )
    Widget w;
    XEvent *event;		/* unused */
    String *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;

#ifdef ARROW_SCROLLBAR
    sbw->scrollbar.scroll_mode = 0;
    /* no need to remove any autoscroll timeout; it will no-op */
    /* because the scroll_mode is 0 */
    /* but be sure to remove timeout in destroy proc */
    /* release all buttons */
    PaintArrows (sbw, 0, 0);
    PaintThumb (sbw, 0);
#else
    XtVaSetValues (w, XtNcursor, sbw->scrollbar.inactiveCursor, NULL);
    XFlush (XtDisplay (w));
    sbw->scrollbar.direction = 0;
#endif
}

static float FractionLoc (sbw, x, y)
    ScrollbarWidget sbw;
    int x, y;
{
    float   result;
    int margin;
    float   height, width;

    margin = MARGIN (sbw);
    x -= margin;
    y -= margin;
    height = sbw->core.height - 2 * margin;
    width = sbw->core.width - 2 * margin;
    result = PICKLENGTH (sbw, x / width, y / height);
    return FloatInRange(result, 0.0, 1.0);
}


static void MoveThumb (w, event, params, num_params)
    Widget w;
    XEvent *event;
    String *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    Position x, y;
    float loc, t, s;

#ifndef ARROW_SCROLLBAR
    if (sbw->scrollbar.direction == 0) return; /* if no StartScroll */
#endif

    if (LookAhead (w, event)) return;

    if (!event->xmotion.same_screen) return;

    ExtractPosition (event, &x, &y);
    loc = FractionLoc (sbw, x, y);
    t = sbw->scrollbar.top;
    s = sbw->scrollbar.shown;
#ifdef ARROW_SCROLLBAR
    if (sbw->scrollbar.scroll_mode != 2 )
      /* initialize picked position */
      sbw->scrollbar.picked = (FloatInRange(loc, t, t+s) - t);
#else
    sbw->scrollbar.picked = 0.5 * s;
#endif
    if (sbw->scrollbar.pick_top)
      sbw->scrollbar.top = loc;
    else {
      sbw->scrollbar.top = loc - sbw->scrollbar.picked;
      if (sbw->scrollbar.top < 0.0) sbw->scrollbar.top = 0.0;
    }
    /* don't allow scrollbar to shrink at end */
    if (sbw->scrollbar.top + sbw->scrollbar.shown > 1.0)
       sbw->scrollbar.top = 1.0 - sbw->scrollbar.shown + 0.001;
#ifdef ARROW_SCROLLBAR
    sbw->scrollbar.scroll_mode = 2; /* indicate continuous scroll */
#endif
    PaintThumb (sbw, 1);
    XFlush (XtDisplay (w));	/* re-draw it before Notifying */
}

/* ARGSUSED */
static void NotifyThumb (w, event, params, num_params )
    Widget w;
    XEvent *event;
    String *params;		/* unused */
    Cardinal *num_params;	/* unused */
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;
    float top = sbw->scrollbar.top;

#ifndef ARROW_SCROLLBAR
    if (sbw->scrollbar.direction == 0) return; /* if no StartScroll */
#endif

    if (LookAhead (w, event)) return;

    /* thumbProc is not pretty, but is necessary for backwards
       compatibility on those architectures for which it work{s,ed};
       the intent is to pass a (truncated) float by value. */
#ifdef ARROW_SCROLLBAR
    /* This corrects for rounding errors: If the thumb is moved to the end of
       the scrollable area sometimes the last line/column is not displayed.
       This can happen when the integer number of the top line or leftmost
       column to be be displayed is calculated from the float value
       sbw->scrollbar.top. The numerical error of this rounding problem is
       very small. We therefore add a small value which then forces the
       next line/column (the correct one) to be used. Since we can expect
       that the resolution of display screens will not be higher then
       10000 text lines/columns we add 1/10000 to the top position. The
       intermediate variable `top' is used to avoid erroneous summing up
       corrections (can this happen at all?). If the arrows are not displayed
       there is no problem since in this case there is always a constant
       integer number of pixels the thumb must be moved in order to scroll
       to the next line/column. */
    top += 0.0001;
#endif
    XtCallCallbacks (w, XtNthumbProc, *(XtPointer*)&top);
    XtCallCallbacks (w, XtNjumpProc, (XtPointer)&top);
}



/************************************************************
 *
 *  Public routines.
 *
 ************************************************************/

/* Set the scroll bar to the given location. */

#if NeedFunctionPrototypes
void XawScrollbarSetThumb (Widget w,
#if NeedWidePrototypes
			double top, double shown)
#else
			float top, float shown)
#endif
#else
void XawScrollbarSetThumb (w, top, shown)
    Widget w;
    float top, shown;
#endif
{
    ScrollbarWidget sbw = (ScrollbarWidget) w;

#ifdef ARROW_SCROLLBAR
    if (sbw->scrollbar.scroll_mode == (char) 2) return; /* if still thumbing */
#else
    if (sbw->scrollbar.direction == 'c') return; /* if still thumbing */
#endif

    sbw->scrollbar.top = (top > 1.0) ? 1.0 :
				(top >= 0.0) ? top : sbw->scrollbar.top;

    sbw->scrollbar.shown = (shown > 1.0) ? 1.0 :
				(shown >= 0.0) ? shown : sbw->scrollbar.shown;

    /* don't allow scrollbar to shrink at end */
    if (sbw->scrollbar.top + sbw->scrollbar.shown > 1.0)
	sbw->scrollbar.top = 1.0 - sbw->scrollbar.shown + 0.001;

    PaintThumb (sbw, 0);
}
