/*
 * $Id: Traversal.c,v 1.3 2000/08/31 17:49:11 falk Exp $
 */


/*

Copyright (c) 1999  X Consortium

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

*/

/*  ___________________________________________________________________
 * /\                                                                  \
 * \_| This code implements keyboard traversal for the Athena widgets. |
 *   |   ______________________________________________________________|_
 *    \_/_______________________________________________________________/
 */



#include "string.h"

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xmu/CharSet.h>
#include <Linux/Xaw95/SimpleP.h>
#include <Linux/Xaw95/TraversalP.h>
#include <Linux/Xaw95/XawDebug.h>



	/* TODO: cache this on a per-display basis */

static	XawFocusPolicy	globalPolicy = XawFocusPointer;


#if NeedFunctionPrototypes
static	Time	getEventTime(XEvent *event);
#else
static	Time	getEventTime();
#endif


static	char	focusTransTab[] =
"   <EnterWindow>:	XawFocusEnterWindow()\n"
"   <LeaveWindow>:	XawFocusLeaveWindow()\n"
"   Shift<Key>Tab:	XawFocusPrevious()\n"
"   <Key>Tab:		XawFocusNext()\n"
"   <Key>Home:		XawFocusHome()\n"
"   <Key>KP_Home:	XawFocusHome()\n"
"   <Key>End:		XawFocusEnd()\n"
"   <Key>KP_End:	XawFocusEnd()\n"
"   <Key>Up:		XawFocusPreviousGroup()\n"
"   <Key>KP_Up:		XawFocusPreviousGroup()\n"
"   <Key>Down:		XawFocusNextGroup()\n"
"   <Key>KP_Down:	XawFocusNextGroup()\n";
static	XtTranslations	focusTrans;
static	Bool		focusInited = False;

static	XtActionsRec	focusActions[] = {
  {"XawFocusNext", XawFocusNextAction},
  {"XawFocusPrevious", XawFocusPreviousAction},
  {"XawFocusHome", XawFocusHomeAction},
  {"XawFocusEnd", XawFocusEndAction},
  {"XawFocusNextGroup", XawFocusNextGroupAction},
  {"XawFocusPreviousGroup", XawFocusPreviousGroupAction},
  {"XawFocusHomeGroup", XawFocusHomeGroupAction},
  {"XawFocusEndGroup", XawFocusEndGroupAction},
  {"XawFocusTake", XawFocusTakeAction},
  {"XawFocusEnterWindow", XawFocusEnterWindowAction},
  {"XawFocusLeaveWindow", XawFocusLeaveWindowAction}
};



	/* Called only once.  Reads the global focusPolicy resource */
void
XawTraversalInit(w)
Widget w;
{
	XtAppContext ctx;
	static Bool firstTime = True;
	if (firstTime) {
		XrmDatabase db = XtScreenDatabase(XtScreen(w));
		char *type;
		XrmValue value;
		Bool rval;
		if (XrmGetResource(db, "focusPolicy", "FocusPolicy", &type, &value) &&
			strcasecmp(type, "string") == 0) {
			if (strcasecmp(value.addr, "old") == 0)
				globalPolicy = XawFocusOld;
			else if (strcasecmp(value.addr, "explicit") == 0)
				globalPolicy = XawFocusExplicit;
			else if (strcasecmp(value.addr, "pointer") == 0)
				globalPolicy = XawFocusPointer;
		}
		firstTime = False;
	}
}



	/* Installs focus actions into application context.  Called only
	 * once per context.
	 */

void XawFocusInstallActions(XtAppContext ctx)
{
	XtAppAddActions(ctx, focusActions, XtNumber(focusActions));
}



	/* Utility:  Adds focus-related translations to a widget. */

void XawFocusInstall(Widget w, Bool override)
{
	if (!focusInited) {
		focusTrans = XtParseTranslationTable(focusTransTab);
		focusInited = True;
	}
	if (override)
		XtOverrideTranslations(w, focusTrans);
	else
		XtAugmentTranslations(w, focusTrans);
}



	/* Utility:  Return visibility status of a widget.  Idea
	 * taken from Motif, source code taken from Lesstif.
	 * Note: the information returned by this function is
	 * subject to change without notice.  This function should
	 * be called from within a server grab if you need to depend
	 * on the results.
	 */

XawVisibility
XawGetVisibility(w)
Widget	w;
{
	XRectangle	rect;

	if (w == NULL)
		return XawVisibilityFullyObscured;

	  /* TODO: finish writing this */
	return True;
}



	/* Internal utility:  Determine if this widget is eligible to accept
	 * keyboard focus.  If it's an Athena subclass, examine the
	 * XtNtraversalOn resource directly.  Otherwise, try to
	 * read the XtNtraversalOn resource via XtGetValues()
	 * (Motif widgets have this resource.)  Otherwise, just
	 * assume True.
	 */

static	Bool
isTraversable(w)
Widget w;
{
	Boolean	traversal = True;

	/* Easy tests first */

	if (w->core.being_destroyed || !XtIsRealized(w) ||
		!XtIsSensitive(w) || !w->core.visible || !XtIsManaged(w))
		return False;

	  /* Is there a resource? */

	if (XtIsSubclass(w, simpleWidgetClass))
		traversal = ((SimpleWidget)w)->simple.traversalOn;
	else
	  /* Note: assumes traversalOn is always Boolean */
		XtVaGetValues(w, XtNtraversalOn, &traversal, 0);
	if (!traversal)
		return False;

	  /* TODO: test for visibility */

	return True;
}



	/* Widget class method.  Accept keyboard focus and return
	 * True.
	 */

Boolean XawAcceptFocus(Widget w, Time *tm)
{
	/* To be eligible for keyboard focus, the widget must be
	 * realized, sensitive, managed, mapped, and visible.
	 * Also check traversalOn resource.
	 * (Unfortunately, there's no reasonable way to test for mapped or
	 * visible -- it requires locking the server and a few round-trips..)
	 */

	if (isTraversable(w)) {
		Widget p;
		for (p = XtParent(w); !XtIsShell(p); p = XtParent(p));
		XtSetKeyboardFocus(p, w);
		return True;
	} else
		return False;
}


void XawFocusNextAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusNext(w, getEventTime(event));
}



void XawFocusPreviousAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusPrevious(w, getEventTime(event));
}


void XawFocusHomeAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	(void)XawFocusHome(w, getEventTime(event));
}


void XawFocusEndAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	(void)XawFocusEnd(w, getEventTime(event));
}


void XawFocusNextGroupAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusNextGroup(w, getEventTime(event));
}

void XawFocusPreviousGroupAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusPreviousGroup(w, getEventTime(event));
}

void XawFocusHomeGroupAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusHomeGroup(w, getEventTime(event));
}


void XawFocusEndGroupAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusEndGroup(w, getEventTime(event));
}


void XawFocusTakeAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	if (globalPolicy == XawFocusOld) return;

	XawFocusTake(w, getEventTime(event));
}


void XawFocusEnterWindowAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	XawFocusEnterWindow(w, globalPolicy, getEventTime(event));
}


void XawFocusLeaveWindowAction(Widget w, XEvent *event, String *params, Cardinal *np) {
	XawFocusLeaveWindow(w, globalPolicy, getEventTime(event));
}

	/* end of the action procs */
#define	lastCh(w)	((w)->composite.num_children - 1)

static	Time
getEventTime(event)
XEvent *event;
{
	Time	tm = CurrentTime;

	if (event != NULL) {
		switch (event->type) {
			case ButtonPress:
			case ButtonRelease: tm = event->xbutton.time; break;
			case KeyPress:
			case KeyRelease: tm = event->xkey.time; break;
			case MotionNotify: tm = event->xmotion.time; break;
			case EnterNotify:
			case LeaveNotify: tm = event->xcrossing.time; break;
		}
	}

	return tm;
}




	/* Utility: search for widget in its parent's child list,
	 * return the index.  Return -1 if not found.
	 */

static	int
findInParent(w)
Widget	w;
{
	CompositeWidget p = (CompositeWidget)XtParent(w);
	int	i, n;

	if (p == NULL)
		return -1;

	for (i = 0; i < p->composite.num_children; ++i)
		if (p->composite.children[i] == w)
			return i;

	return -1;
}




	/* utility: search composite widget w, starting at c, for
	 * a child widget willing to accept input.  Direction is
	 * either +1 or -1 for forward/backward search.  If composite
	 * widgets are willing to accept keyboard traversal, descend
	 * into them.
	 */


static	Bool
focusFind(w, c, recursive, direction, tm)
CompositeWidget	w;
int	c;
Bool	recursive;
int	direction;
Time	tm;
{
	int	i, nc;
	Widget	cw;

	nc = w->composite.num_children;

	while (c >= 0 && c < nc) {
		cw = w->composite.children[c];
		if (XtIsManaged(cw)) {
			if (XtCallAcceptFocus(cw, &tm)) {
				debugf("focus passed to %s\n", XtName(cw));
				return True;
			}
			if (recursive && XtIsComposite(cw) && isTraversable(cw)) {
				CompositeWidget cwc = (CompositeWidget)cw;
				int c0 = direction > 0 ? 0 : lastCh(cwc);
				if (focusFind(cwc, c0, recursive, direction, tm))
					return True;
			}
		}
		c += direction;
	}

	return False;	/* nobody wanted it */
}



	/* Find next widget in this widget tree.  This can involve
	 * searching up or down in the tree.
	 */
void
XawFocusNext(w, tm)
Widget	w;
Time	tm;
{
	Widget	p;
	int	ci;

	/* Search for the next widget after this one to
	 * receive input.  If we encounter a composite widget,
	 * set focus to its first willing child, if any.  If we
	 * reach the end, pop up a level and keep trying.  If we
	 * can't pop up, go to the start.
	 */

	/* First, see if this is a composite widget; test children. */

	if (XtIsComposite(w) &&
		focusFind((CompositeWidget)w, 0, True, +1, tm))
		return;

	  /* Nope.  Start searching siblings.  Easiest way to do this
	   * is to search parent.
	   */

	while ((p = XtParent(w)) != NULL && !XtIsShell(p)) {
		if ((ci = findInParent(w)) == -1)
			return;

		if (focusFind((CompositeWidget)p, ci + 1, True, +1, tm))
			return;

		  /* No luck there; pop up a level and try again */
		w = p;
	}

	/* Made it all the way to the top.  Start from the beginning */
	XawFocusHome(w, tm);
}


	/* Utility: search parents until a parent with multiple children
	 * is found.  Also returns the index of the next-to-top parent.
	 * This may be the original widget itself if the search terminates
	 * immediately, as it usually does.
	 */

static	CompositeWidget
getMultiParent(w, idx)
Widget	w;
int *idx;
{
	CompositeWidget p;
	while ((p = (CompositeWidget)XtParent(w)) != NULL &&
		!XtIsShell((Widget)p) &&
		p->composite.num_children == 1)
		w = (Widget)p;
	*idx = findInParent(w);
	return p;
}


	/* Find next widget at this level in the tree. */

void
XawFocusNextGroup(w, tm)
Widget	w;
Time	tm;
{
	CompositeWidget p;
	int	ci;

	/* Search this widget's parent for another widget eligible
	 * to receive input.
	 *
	 * Actually, if the parent has only this one child, traversal
	 * isn't very interesting, so we first search upwards until
	 * we find a multi-child parent.
	 */

	if ((p = getMultiParent(w, &ci)) == NULL)
		return;

	if (ci != -1) {
		if (focusFind(p, ci + 1, False, +1, tm))
			return;
		if (focusFind(p, 0, False, +1, tm))
			return;
	}
	/* nobody wanted it, just return */
}


	/* Find previous widget in the tree */

void
XawFocusPrevious(w, tm)
Widget	w;
Time	tm;
{
	Widget	p;
	int	ci;

	/* see focusNext() for detailed comments */

	while ((p = XtParent(w)) != NULL && !XtIsShell(p)) {
		if ((ci = findInParent(w)) == -1) return;
		if (focusFind((CompositeWidget)p, ci - 1, True, -1, tm)) return;
		w = (Widget)p;
	}
	XawFocusEnd(w, tm);
}

	/* Find previous widget at this level of the tree */

void
XawFocusPreviousGroup(w, tm)
Widget	w;
Time	tm;
{
	CompositeWidget p;
	int	ci;

	if ((p = getMultiParent(w, &ci)) == NULL) return;

	if (ci != -1) {
		if (focusFind(p, ci - 1, False, -1, tm)) return;
		if (focusFind(p, lastCh(p), False, -1, tm)) return;
	}
}

	/* Find first widget in the tree */

void
XawFocusHome(w, tm)
Widget	w;
Time	tm;
{
	Widget	p;

	/* Climb up to the top-level widget and then start
	 * searching from there.
	 */

	while ((p = XtParent(w)) != NULL && !XtIsShell(p))
		w = p;

	(void)focusFind((CompositeWidget)w, 0, True, +1, tm);
}

	/* Find first widget at this level of the tree */
void
XawFocusHomeGroup(w, tm)
Widget	w;
Time	tm;
{
	int	ci;
	CompositeWidget p = getMultiParent(w, &ci);

	if (p != NULL)
		(void)focusFind((CompositeWidget)p, 0, True, +1, tm);
}

	/* Find last widget in the tree */
void
XawFocusEnd(w, tm)
Widget	w;
Time	tm;
{
	Widget	p;

	while ((p = XtParent(w)) != NULL && !XtIsShell(p))
		w = p;

	(void)focusFind((CompositeWidget)w,
		lastCh(((CompositeWidget)w)), True, -1, tm);
}

	/* Find last widget at this level in the tree */
void
XawFocusEndGroup(w, tm)
Widget	w;
Time	tm;
{
	int	ci;
	CompositeWidget p = getMultiParent(w, &ci);

	if (p != NULL)
		(void)focusFind((CompositeWidget)w,
			lastCh(((CompositeWidget)w)),
			True, -1, tm);
}

	/* Focus goes to this widget */
void
XawFocusTake(w, tm)
Widget	w;
Time	tm;
{
	(void)XtCallAcceptFocus(w, &tm);
}

	/* Handle pointer entering the widget.  Action taken depends on
	 * focus policy.
	 */
void
XawFocusEnterWindow(w, policy, tm)
Widget	w;
XawFocusPolicy policy;
Time	tm;
{
	Widget	p;

	if (policy == XawFocusDefault) policy = globalPolicy;
	switch (policy) {
		case XawFocusExplicit: break;
		case XawFocusPointer:
		case XawFocusOld:
			XawFocusTake(w, tm);
			break;
	}
}

	/* Handle pointer leaving the widget.  Action taken depends
	 * on focus policy.
	 */
void
XawFocusLeaveWindow(w, policy, tm)
Widget	w;
XawFocusPolicy policy;
Time	tm;
{
	Widget	p;

	if (policy == XawFocusDefault) policy = globalPolicy;
	switch (policy) {
		case XawFocusExplicit: break;
		case XawFocusPointer:
		case XawFocusOld:
			for (p = XtParent(w); !XtIsShell(p); p = XtParent(p));
			XtSetKeyboardFocus(p, None);
#ifdef	COMMENT
			XSetInputFocus(XtDisplay(w), PointerRoot, RevertToPointerRoot, tm);
#endif	/* COMMENT */
			break;
	}
}
