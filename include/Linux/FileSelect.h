/* $Id: FileSelect.h,v 1.2 1999/03/01 08:57:41 falk Exp $
 *
 *	File selection browser.  These functions help you create and
 *	use a standard file selection browser using Athena widgets.
 *
 *	Best if viewed with Xaw3d or better.
 *
 *
 * Functions provided here:
 *
 *
 *  void
 *  FileSelectSet(Widget fileSelect, String dir, String filter, String filename)
 *
 *	Sets the initial search parameters for the browser.  Directory,
 *	filter and/or filename may be NULL, in which case they will be
 *	unchanged.
 *
 *  FileSelectGetFilename( Widget fileSelect, char path[MAXPATHLEN] )
 *
 *	Gets the current value of the path/filename as entered by the
 *	user.  This may not be meaningful if the user has not selected
 *	a file.
 *
 *  Widget
 *  FileSelectCreateWindow( String name, Widget parent,
 *			XtCallback cb, XtPointer client )
 *
 *	Creates a top-level transient window containing a single FileSelect
 *	widget.  If cb is non-null, it is added to the FileSelect's callback
 *	list.  This function returns the file select widget.
 *	Name of parent widget is "<name>_win"
 *
 *  void
 *  FileSelectDestroyWindow( Widget fileSelect )
 *
 *  void
 *  FileSelectPopup( Widget fileSelect )
 *
 *  void
 *  FileSelectPopdown( Widget fileSelect )
 *
 *	Manipulate file select transient window.  Argument is the
 *	FileSelect widget, not the parent.
 *
 *
 *  Widget
 *  FileSelectParent( Widget fileSelect )
 *	Returns the top-level shell widget for this fileSelect widget.
 *
 */

#ifndef	_FileSelect_h
#define	_FileSelect_h

#include <X11/Intrinsic.h>

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 dirLabel	     Label		String		"Directory:"
 fileLabel	     Label		String		"File:"
 filterLabel	     Label		String		"File types:"
 openLabel	     Label		String		"Open"
 cancelLabel	     Label		String		"Cancel"
 callback	     Callback		Pointer		None +
 clickTime	     ClickTime		int		500 +

 defaultDistance     Thickness		int		2
 background	     Background		Pixel		XtDefaultBackground
 border		     BorderColor	Pixel		XtDefaultForeground
 borderWidth	     BorderWidth	Dimension	1
 destroyCallback     Callback		Pointer		NULL
 height		     Height		Dimension	computed at create
 icon		     Icon		Pixmap		0
 label		     Label		String		NULL
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 sensitive	     Sensitive		Boolean		True
 value		     Value		String		NULL
 width		     Width		Dimension	computed at create
 x		     Position		Position	0
 y		     Position		Position	0

+ callback is called when the user presses "Open" or "Cancel".  If "Open",
  call_data is the full pathname selected by user.  If "Cancel", call_data
  is NULL.

+ clicktime is the timeout for double-clicks.

*/

#ifndef	XtNfilterLabel
#define	XtNfilterLabel	"filterLabel"
#endif

#ifndef	XtNdirLabel
#define	XtNdirLabel	"dirLabel"
#endif

#ifndef	XtNfileLabel
#define	XtNfileLabel	"fileLabel"
#endif

#ifndef	XtNselectionLabel
#define	XtNselectionLabel	"selectionLabel"
#endif

#ifndef	XtNopenLabel
#define	XtNopenLabel	"openLabel"
#endif

#ifndef	XtNcancelLabel
#define	XtNcancelLabel	"cancelLabel"
#endif

#ifndef	XtNclickTime
#define	XtNclickTime	"clickTime"
#define	XtCClickTime	"ClickTime"
#endif

typedef struct _FileSelectClassRec *FileSelectWidgetClass;
typedef struct _FileSelectRec *FileSelectWidget;

extern WidgetClass fileSelectWidgetClass;

_XFUNCPROTOBEGIN

#if	NeedFunctionPrototypes

extern	void	FileSelectSet(
	Widget	fileSelect,
	String	directory,
	String	filter,
	String	filename
);

extern void FileSelectGetFilename(
	Widget,		/* fileSelect */
	String		/* result */
);

extern	Widget	FileSelectCreateWindow(
	String	name,		/* file select widget name */
	Widget	parent,		/* parent window shell */
	XtCallbackProc cb,		/* optional callback proc */
	XtPointer	client		/* optional callback data */
);

extern	void	FileSelectDestroyWindow(Widget fileSelect);
extern	void	FileSelectPopup(Widget fileSelect);
extern	void	FileSelectPopdown(Widget fileSelect);
extern	Widget	FileSelectParent(Widget fileSelect);

#else

extern	void	FileSelectSet();
extern	String	FileSelectGetFilename();
extern	Widget	FileSelectCreateWindow();
extern	void	FileSelectDestroyWindow();
extern	void	FileSelectPopup();
extern	void	FileSelectPopdown();
extern	Widget	FileSelectParent();

#endif	/* NeedFunctionPrototypes */

_XFUNCPROTOEND

#endif /* _XawFileSelect_h */
