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

#ifndef s_Html_h
#define s_Html_h

#include <X11/IntrinsicP.h>
#include "MwHtmlParser.h"
#include "image.h"

/****************************************************************
 *
 * Html widget
 *
 ****************************************************************/

/* Resources:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	0
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 width		     Width		Dimension	0
 x		     Position		Position	0
 y		     Position		Position	0
 zoom		     Float		Float		1.0

*/

/* define any special resource names here that are not in <X11/StringDefs.h> */

#ifndef XtNurl
#define XtNurl "url"
#define XtCUrl "Url"
#endif
#ifndef XtNzoom
#define XtNzoom "zoom"
#define XtCZoom "Zoom"
#endif
#ifndef XtNdelay
#define XtNdelay "delay"
#define XtCDelay "Delay"
#endif
#ifndef XtNtopRow
#define XtNtopRow "topRow"
#define XtCTopRow "TopRow"
#endif
#ifndef XtNtopCol
#define XtNtopCol "topCol"
#define XtCTopCol "TopCol"
#endif
#ifndef XtNtotalHeight
#define XtNtotalHeight "totalHeight"
#define XtCTotalHeight "TotalHeight"
#endif
#ifndef XtNtotalWidth
#define XtNtotalWidth "totalWidth"
#define XtCTotalWidth "TotalWidth"
#endif
#ifndef XtNstatus
#define XtNstatus "status"
#define XtCStatus "Status"
#endif
#ifndef XtNchangeUrl
#define XtNchangeUrl "changeUrl"
#define XtCChangeUrl "ChangeUrl"
#endif

/* constraint parameters */
#ifndef XtNhtmlx
#if 0
#define XtNhtmlx	"htmlx"
#define XtCHtmlx	"Htmlx"
#define XtNhtmly	"htmly"
#define XtCHtmly	"Htmly"
#define XtNhtmlw	"htmlw"
#define XtCHtmlw	"Htmlw"
#define XtNhtmlh	"htmlh"
#define XtCHtmlh	"Htmlh"
#endif
#define XtNhtmlObject	"htmlObject"
#define XtCHtmlObject	"HtmlObject"
#endif

/* declare specific HtmlWidget class and instance datatypes */

typedef struct s_HtmlClassRec*	MwHtmlWidgetClass;
typedef struct s_HtmlRec*		MwHtmlWidget;

/* declare the class constant */

extern WidgetClass mwHtmlWidgetClass;

#endif /* s_Html_h */
