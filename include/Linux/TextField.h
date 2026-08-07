/*-----------------------------------------------------------------------------
 * TextField	A single line text entry widget
 *
 * Copyright (c) 1995 Robert W. McMullen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Author: Rob McMullen <rwmcm@orion.ae.utexas.edu>
 *         http://www.ae.utexas.edu/~rwmcm
 */

#ifndef _TextField_H
#define _TextField_H

#include <X11/Intrinsic.h>

#define _TextField_WIDGET_VERSION	1.0

#ifndef XtIsTextField
#define XtIsTextField(w) XtIsSubclass((Widget)w, textfieldWidgetClass)
#endif

/* Athena style resource names */

#ifndef XtNecho
#define XtNecho			"echo"
#endif
#ifndef XtNpendingDelete
#define XtNpendingDelete	"pendingDelete"
#endif
#ifndef XtNlength
#define XtNlength		"length"
#endif
#ifndef XtNstring
#define XtNstring		"string"
#endif
#ifndef XtNinsertPosition
#define XtNinsertPosition	"insertPosition"
#endif
#ifndef XtNdisplayCaret
#define XtNdisplayCaret		"displayCaret"
#endif
#ifndef XtNeditable
#define XtNeditable		"editable"
#endif
#define XtNmargin		"margin"
#define XtNcursorWidth		"cursorWidth"
#define XtNallowSelection	"allowSelection"
#define XtNactivateCallback	"activateCallback"
#define	XtNvalueChangedCallback	"valueChangedCallback"
#define	XtNfocusCallback	"focusCallback"
#define	XtNlosingFocusCallback	"losingFocusCallback"
#define	XtNgainPrimaryCallback	"gainPrimaryCallback"
#define	XtNlosePrimaryCallback	"losePrimaryCallback"
#define	XtNmodifyVerifyCallback	"modifyVerifyCallback"
#define	XtNmotionVerifyCallback	"motionVerifyCallback"


/* Motif style resource names */

#ifndef XmNmaxLength
#define XmNmaxLength		XtNlength
#endif
#ifndef XmNvalue
#define XmNvalue		XtNvalue
#endif
#ifndef XmNcursorPosition
#define XmNcursorPosition	XtNinsertPosition
#endif
#ifndef XmNcursorPositionVisible
#define XmNcursorPositionVisible	XtNdisplayCaret
#endif
#ifndef XmNeditable
#define XmNeditable		XtNeditable
#endif
#ifndef XmNactivateCallback
#define XmNactivateCallback	XtNactivateCallback
#endif
#ifndef XmNfocusCallback
#define XmNfocusCallback	XtNfocusCallback
#endif
#ifndef XmNgainPrimaryCallback
#define XmNgainPrimaryCallback	XtNgainPrimaryCallback
#endif
#ifndef XmNlosePrimaryCallback
#define XmNlosePrimaryCallback	XtNlosePrimaryCallback
#endif
#ifndef XmNlosingFocusCallback
#define XmNlosingFocusCallback	XtNlosingFocusCallback
#endif
#ifndef XmNmodifyVerifyCallback
#define XmNmodifyVerifyCallback	XtNmodifyVerifyCallback
#endif
#ifndef XmNmotionVerifyCallback
#define XmNmotionVerifyCallback	XtNmotionVerifyCallback
#endif
#ifndef XmNvalueChangedCallback
#define XmNvalueChangedCallback	XtNvalueChangedCallback
#endif


extern WidgetClass textfieldWidgetClass;

typedef struct _TextFieldClassRec *TextFieldWidgetClass;
typedef struct _TextFieldRec *TextFieldWidget;

typedef struct _TextFieldReturnStruct {
	int	reason;		/* Motif compatibility */
	XEvent *event;
	char *string;
} TextFieldReturnStruct;

typedef	struct {
	char *ptr;
	int	length;
	int	format;	/* reserved; Motif compatibility */
} TextFieldBlockRec, *TextFieldBlock;

typedef	struct _TextFieldVerifyStruct {
	int	reason;	/* Motif compatibility */
	XEvent *event;
	Boolean	doit;
	int	curInsert, newInsert;
	int	startPos, endPos;
	TextFieldBlock text;
} TextFieldVerifyStruct;

	/* reason values: (Motif compatibility) */

#define	TF_VALUE_CHANGED	2
#define	TF_ACTIVATE		10
#define	TF_FOCUS		18
#define	TF_LOSING_FOCUS		19
#define	TF_MODIFYING_TEXT_VALUE	20
#define	TF_MOVING_INSERT_CURSOR	21
#define	TF_GAIN_PRIMARY		41
#define	TF_LOSE_PRIMARY		42

/*
** Public function declarations
*/
#if __STDC__ || defined(__cplusplus)
#define P_(s) s
#else
#define P_(s) ()
#endif

/* TextField.c */
Boolean TextFieldGetEditable P_((Widget aw));
int TextFieldGetInsertionPosition P_((Widget aw));
char *TextFieldGetString P_((Widget aw));
void TextFieldInsert P_((Widget aw, int pos, char *str));
void TextFieldReplace P_((Widget aw, int first, int last, char *str));
void TextFieldSetEditable P_((Widget aw, Boolean editable));
void TextFieldSetInsertionPosition P_((Widget aw, int pos));
void TextFieldSetSelection P_((Widget aw, int start, int end, Time time));
void TextFieldSetString P_((Widget aw, char *str));

#undef P_

#endif /* _TextField_H */
