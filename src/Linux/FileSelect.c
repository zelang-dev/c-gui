/*
 * $Id: FileSelect.c,v 1.6 1999/03/29 08:14:40 falk Exp $
 *
 * Edward A. Falk, falk@falconer.vip.best.com
 *
 * TODO:
 *	Keystrokes in dirText or fileName match file list.
 *	Tab in dirText or fileName execute completion.
 *	Implement "new directory" button.
 *	Relative pathnames in directory text widget
 *	Return while scrolling through directory names to select dir
 */

#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include <X11/IntrinsicP.h>
#include <X11/Xos.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Misc.h>
#include <Linux/Xaw95/AsciiText.h>
#include <Linux/Xaw95/Command.h>
#include <Linux/Xaw95/MenuButton.h>
#include <Linux/Xaw95/Label.h>
#include <Linux/Xaw95/Viewport.h>
#include <Linux/Xaw95/List.h>
#include <Linux/Xaw95/Scrollbar.h>
#include <Linux/Xaw95/SimpleMenu.h>
#include <Linux/Xaw95/SmeBSB.h>
#include <Linux/Xaw95/SmeLine.h>
#include <stdlib.h>

#include "FileSelectP.h"

#define	MAX_MENU	20

/*
 * After we have set the string in the value widget we set the
 * string to a magic value.  So that when a SetValues request is made
 * on the dialog value we will notice it, and reset the string.
 */



#define streq(a,b) ((a)==(b) || strcmp( (a), (b) ) == 0)
#define	MALLOC(t,n)		((t *)XtMalloc((n)*sizeof(t)))
#define	REALLOC(ptr,t,n) \
	((ptr)=(t *)XtRealloc((char *)(ptr), (n)*sizeof(t)))

#ifndef	min
#define	min(a,b)	((a)<(b)?(a):(b))
#define	max(a,b)	((a)>(b)?(a):(b))
#endif



	/* FileSelect translations & accelerators */

static	char	defaultTrans[] = "#override\
   <Key>Page_Up:	FileScroll(pgup)\n\
   <Key>KP_Page_Up:	FileScroll(pgup)\n\
   <Key>Page_Down:	FileScroll(pgdn)\n\
   <Key>KP_Page_Down:	FileScroll(pgdn)\n\
   <Key>Up:		FileScroll(up)\n\
   <Key>KP_Up:		FileScroll(up)\n\
   <Key>Down:		FileScroll(down)\n\
   <Key>KP_Down:	FileScroll(down)\n\
   <Key>Home:		FileScroll(home)\n\
   <Key>KP_Home:	FileScroll(home)\n\
   <Key>End:		FileScroll(end)\n\
   <Key>KP_End:		FileScroll(end)\n\
   <Key>Tab:		FileExpand()\n\
   <Key>KP_Tab:		FileExpand()";
static	XtAccelerators	accelTbl;


	/* Open button accelerators */

static	char	openAccelerators[] = "#override\
	<Key>Return:	set() notify() unset()\n\
	<Key>Linefeed:	set() notify() unset()";
static	XtAccelerators	openAccelTbl;


	/* Cancel button accelerators */

static	char	cancelAccelerators[] = "#override\
	<Key>Escape:	set() notify() unset()	";
static	XtAccelerators	cancelAccelTbl;


	/* transations to add to directory name widget */
	/* TODO: filename completion per keystroke */

static	char	dirTrans[] =
"    <Key>Return:	__FileSelectDir()\n\
     <Key>Linefeed:	__FileSelectDir()\n\
     <Key>Tab:		__FileSelectDir()";
static	XtTranslations	dirTransTbl;



	/* transations to add to filter widget */

static	char	filterTrans[] =
"    <Key>Return:	__FileSelectFilter()\n\
     <Key>Linefeed:	__FileSelectFilter()\n\
     <Key>Tab:		__FileSelectFilter()";
static	XtTranslations	filterTransTbl;



#define	offset(field)	XtOffsetOf(FileSelectRec, fileSelect.field)

static XtResource resources[] = {
  {XtNdirLabel, XtCLabel, XtRString, sizeof(String),
	 offset(dirLabel), XtRString, "Directory:"},
  {XtNfileLabel, XtCLabel, XtRString, sizeof(String),
	 offset(fileLabel), XtRString, "File:"},
  {XtNopenLabel, XtCLabel, XtRString, sizeof(String),
	 offset(openLabel), XtRString, "Open"},
  {XtNcancelLabel, XtCLabel, XtRString, sizeof(String),
	 offset(cancelLabel), XtRString, "Cancel"},
  {XtNfilterLabel, XtCLabel, XtRString, sizeof(String),
	 offset(filterLabel), XtRString, "File types:"},
  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
	 offset(callbacks), XtRCallback, NULL},
  {XtNclickTime, XtCClickTime, XtRInt, sizeof(int),
	 offset(clickTime), XtRImmediate, (XtPointer)500},
  {XtNdefaultDistance, XtCThickness, XtRInt, sizeof(int),
	 XtOffsetOf(FileSelectRec,gridbox.defaultDistance),
	 XtRImmediate, (XtPointer)2},
};

#undef	offset



static void FileSelectClassInit(void);
static void FileSelectInit(Widget request, Widget new, ArgList args, Cardinal *num_args);
static void FileSelectDestroy(Widget w);

static void fileSelectUpCB(Widget txt, XtPointer client, XtPointer call_data);
static void fileSelectNewCB(Widget txt, XtPointer client, XtPointer call_data);
static void fileSelectRereadCB(Widget txt, XtPointer client, XtPointer call_data);
static void fileSelectOpenCB(Widget txt, XtPointer client, XtPointer call_data);
static void fileSelectCancelCB(Widget txt, XtPointer client, XtPointer call_data);
static void fileCB(Widget txt, XtPointer client, XtPointer call_data);
static void menuSelectCB(Widget txt, XtPointer client, XtPointer call_data);
static void fileKey(Widget txt, XtPointer client, XtPointer call_data);

static FileSelectWidget getFsWidget(Widget w);

static void scrollTo(FileSelectWidget fw, int idx, Widget clip);
static void fileSelectListDir(FileSelectWidget fw);
static Boolean fileSelectMatchFile(String file, String pattern);
static Boolean fileSelectMatchFile2(String file, String *pattern, int np);
static Widget makeTextField(char *name, Widget parent);
static void createbitmap(Widget w, int wid, int hgt, unsigned char *bits);
static void appendDir(FileSelectWidget fw, String dir);
static void upDir(FileSelectWidget fw, String dir);
static int fileComplete(FileSelectWidget fw);
static void addMenuItem(FileSelectWidget fw, char *name, char *value);
static void addMenuMaybe(FileSelectWidget fw, String dir);

	/* Actions */
static void FileSelectFilter(Widget w, XEvent *event, String *params, Cardinal *num_params);
static void FileSelectDir(Widget w, XEvent *event, String *params, Cardinal *num_params);
static void FileScroll(Widget w, XEvent *event, String *params, Cardinal *num_params);
static void FileExpand(Widget w, XEvent *event, String *params, Cardinal *num_params);

	/* Actions added to global actions table so that they may
	 * be referenced by translation tables above
	 */
static XtActionsRec actionsList[] = {
  {"__FileSelectFilter", FileSelectFilter},
  {"__FileSelectDir", FileSelectDir},
};

	/* FileSelect widget actions that may be referenced by
	 * accelerators.
	 */
static XtActionsRec fsActionsList[] = {
  {"FileScroll",	FileScroll},
  {"FileExpand",	FileExpand},
};

#define	SUPERCLASS	((WidgetClass) &gridboxClassRec)

FileSelectClassRec fileSelectClassRec = {
  { /* core_class fields */
		/* superclass	*/	SUPERCLASS,
		/* class_name	*/	"FileSelect",
		/* widget_size	*/	sizeof(FileSelectRec),
		/* class_initialize	*/	FileSelectClassInit,
		/* class_part init	*/	NULL,
		/* class_inited	*/	FALSE,
		/* initialize	*/	FileSelectInit,
		/* initialize_hook	*/	NULL,
		/* realize		*/	XtInheritRealize,
		/* actions		*/	fsActionsList,
		/* num_actions	*/	XtNumber(fsActionsList),
		/* resources	*/	resources,
		/* num_resources	*/	XtNumber(resources),
		/* xrm_class	*/	NULLQUARK,
		/* compress_motion	*/	TRUE,
		/* compress_exposure  */	TRUE,
		/* compress_enterleave*/	TRUE,
		/* visible_interest	*/	FALSE,
		/* destroy		*/	FileSelectDestroy,
		/* resize		*/	XtInheritResize,
		/* expose		*/	XtInheritExpose,
		/* set_values	*/	NULL,
		/* set_values_hook	*/	NULL,
		/* set_values_almost*/	XtInheritSetValuesAlmost,
		/* get_values_hook	*/	NULL,
		/* accept_focus	*/	NULL,
		/* version		*/	XtVersion,
		/* callback_private	*/	NULL,
		/* tm_table		*/	defaultTrans,
		/* query_geometry	*/	XtInheritQueryGeometry,
		/* display_accelerator*/	XtInheritDisplayAccelerator,
		/* extension	*/	NULL
	  },
	  { /* composite_class fields */
		  /* geometry_manager	*/	XtInheritGeometryManager,
		  /* change_managed	*/	XtInheritChangeManaged,
		  /* insert_child	*/	XtInheritInsertChild,
		  /* delete_child	*/	XtInheritDeleteChild,
		  /* extension	*/	NULL
		},
		{ /* constraint_class fields */
			/* subresourses	*/	NULL,
			/* subresource_count*/	0,
			/* constraint_size	*/	sizeof(FileSelectConstraintsRec),
			/* initialize	*/	NULL,
			/* destroy		*/	NULL,
			/* set_values	*/	NULL,
			/* extension	*/	NULL
		  },
		  { /* gridbox_class fields*/
			  /* extension	*/	NULL,
			},
			{ /* fileSelect_class fields */
				/* extension	*/	NULL
			  }
};

WidgetClass fileSelectWidgetClass = (WidgetClass)&fileSelectClassRec;

typedef	struct fsinfo {
	mode_t mode;
	String name;
} FSinfo;

/* Built-in Bitmaps */
#define dnarrow_width 11
#define dnarrow_height 7
static unsigned char dnarrow_bits[] = {
   0x00, 0x00, 0xfe, 0x03, 0xfc, 0x01, 0xf8, 0x00, 0x70, 0x00, 0x20, 0x00,
   0x00, 0x00};

#define folder_new_width 16
#define folder_new_height 16
static unsigned char folder_new_bits[] = {
   0x00, 0x00, 0x00, 0x10, 0x00, 0x92, 0xf0, 0x54, 0x08, 0x01, 0xfc, 0xcf,
   0x02, 0x10, 0x02, 0x54, 0x02, 0x92, 0x02, 0x10, 0x02, 0x10, 0x02, 0x10,
   0x02, 0x10, 0xfe, 0x1f, 0x00, 0x00, 0x00, 0x00};

#define folder_up_width 16
#define folder_up_height 16
static unsigned char folder_up_bits[] = {
   0x00, 0x00, 0xf0, 0x00, 0x08, 0x01, 0xfc, 0x3f, 0x02, 0x40, 0x22, 0x40,
   0x72, 0x40, 0xfa, 0x40, 0x22, 0x40, 0x22, 0x40, 0xc2, 0x4f, 0x02, 0x40,
   0x02, 0x40, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00};

#ifdef	COMMENT
#define reread_width 16
#define reread_height 16
static unsigned char reread_bits[] = {
   0x00, 0x00, 0xc0, 0x01, 0xf0, 0x07, 0x78, 0x0f, 0x1c, 0x1c, 0x0c, 0x18,
   0x0e, 0x38, 0x06, 0x30, 0x0e, 0x00, 0xcc, 0x00, 0xdc, 0x01, 0xf8, 0x03,
   0xf0, 0x07, 0xc0, 0x03, 0xc0, 0x01, 0xc0, 0x00};
#endif	/* COMMENT */
#define reread_width 16
#define reread_height 16
static unsigned char reread_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x07, 0xe4, 0x1f, 0x3c, 0x18,
   0x3c, 0x30, 0x2c, 0x20, 0x7c, 0x20, 0x00, 0x20, 0x00, 0x20, 0x00, 0x10,
   0x00, 0x08, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00};

static void FileSelectClassInit(void) {
	openAccelTbl = XtParseAcceleratorTable(openAccelerators);
	cancelAccelTbl = XtParseAcceleratorTable(cancelAccelerators);
	accelTbl = XtParseAcceleratorTable(defaultTrans);
	dirTransTbl = XtParseTranslationTable(dirTrans);
	filterTransTbl = XtParseTranslationTable(filterTrans);
}

/* ARGSUSED */
static void FileSelectInit(Widget request, Widget new, ArgList args, Cardinal *num_args) {
	FileSelectWidget fw = (FileSelectWidget)new;
	Widget	row1, row3, dbox, frame;
	Widget	dirLabel, fileLabel, filterLabel;
	Widget	fileScroll;
	Widget	openButton, cancelButton;
	Widget	textSrc;
	String	emptyList[1] = {NULL};
	char	path[MAXPATHLEN];
	char *ptr;

	fw->fileSelect.fileNames = NULL;
	fw->fileSelect.fileInfo = NULL;
	fw->fileSelect.nfiles = fw->fileSelect.nfalloc = 0;
	fw->fileSelect.nmenu = 0;

	row1 = XtVaCreateManagedWidget("row1", gridboxWidgetClass, new,
		XtNdefaultDistance, 2,
		XtNborderWidth, 0,
		XtNgridx, 0,
		XtNgridy, 0,
		XtNweightx, 1,
		0);

	dirLabel = XtVaCreateManagedWidget("dirLabel", labelWidgetClass, row1,
		XtNborderWidth, 0,
		XtNlabel, fw->fileSelect.dirLabel,
		XtNgridx, 0,
		XtNgridy, 0,
		XtNfill, FillNone,
		0);

	dbox = XtVaCreateManagedWidget("dbox", gridboxWidgetClass, row1,
		XtNborderWidth, 1,
		XtNdefaultDistance, 0,
		XtNgridx, 1,
		XtNgridy, 0,
		XtNweightx, 1,
		XtNfill, FillWidth,
		0);

	fw->fileSelect.dirName = makeTextField("dirname", dbox);
	XtVaSetValues(fw->fileSelect.dirName,
		XtNborderWidth, 0,
		XtNgridx, 0,
		XtNgridy, 0,
		XtNweightx, 1,
		0);

	fw->fileSelect.dirButton = XtVaCreateManagedWidget("dirButton",
		menuButtonWidgetClass, dbox,
		XtNmenuName, "dirMenu",
		XtNgridx, 1,
		XtNgridy, 0,
		0);

	fw->fileSelect.upButton = XtVaCreateManagedWidget("upButton",
		commandWidgetClass, row1,
		XtNgridx, 2,
		XtNgridy, 0,
		XtNfill, FillNone,
		0);
	XtAddCallback(fw->fileSelect.upButton, XtNcallback, fileSelectUpCB, NULL);

	fw->fileSelect.newButton = XtVaCreateManagedWidget("newButton",
		commandWidgetClass, row1,
		XtNgridx, 3,
		XtNgridy, 0,
		XtNfill, FillNone,
		XtNsensitive, False,		/* TODO */
		0);
	XtAddCallback(fw->fileSelect.newButton, XtNcallback, fileSelectNewCB, NULL);

	fw->fileSelect.rescanButton = XtVaCreateManagedWidget("rescanButton",
		commandWidgetClass, row1,
		XtNgridx, 4,
		XtNgridy, 0,
		XtNfill, FillNone,
		0);
	XtAddCallback(fw->fileSelect.rescanButton,
		XtNcallback, fileSelectRereadCB, NULL);

	/* next row */
	fw->fileSelect.fileScroll = fileScroll =
		XtVaCreateManagedWidget("filesScroll",
			viewportWidgetClass, new,
			XtNheight, 100,
			XtNallowHoriz, True,
			XtNallowVert, True,
			XtNforceBars, False,
			XtNgridx, 0,
			XtNgridy, 1,
			XtNweightx, 1,
			XtNweighty, 1,
			0);

	fw->fileSelect.fileList = XtVaCreateManagedWidget("filesList",
		listWidgetClass, fileScroll,
		XtNborderWidth, 1,
		XtNlist, emptyList,
		XtNdefaultColumns, 1,
		XtNforceColumns, True,
		0);

	XtAddCallback(fw->fileSelect.fileList, XtNcallback, fileCB, (XtPointer)fw);
	row3 = XtVaCreateManagedWidget("row3", gridboxWidgetClass, new,
		XtNdefaultDistance, 2,
		XtNborderWidth, 0,
		XtNgridx, 0,
		XtNgridy, 2,
		XtNweightx, 1,
		0);

		/* next row */
	fileLabel = XtVaCreateManagedWidget("fileLabel", labelWidgetClass, row3,
		XtNborderWidth, 0,
		XtNlabel, fw->fileSelect.fileLabel,
		XtNjustify, XtJustifyRight,
		XtNgridx, 0,
		XtNgridy, 0,
		XtNfill, FillWidth,
		0);

	fw->fileSelect.fileName = makeTextField("selectionText", row3);
	XtVaSetValues(fw->fileSelect.fileName,
		XtNwidth, 250,
		XtNgridx, 1,
		XtNgridy, 0,
		XtNweightx, 1,
		XtNfill, FillWidth,
		0);

	XtVaGetValues(fw->fileSelect.fileName, XtNtextSource, &textSrc, 0);
	XtAddCallback(textSrc, XtNcallback, fileKey, (XtPointer)fw);
	fw->fileSelect.openButton = openButton =
		XtVaCreateManagedWidget("openButton",
			commandWidgetClass, row3,
			XtNlabel, fw->fileSelect.openLabel,
			XtNgridx, 2,
			XtNgridy, 0,
			XtNfill, FillWidth,
			0);
	XtAddCallback(openButton, XtNcallback, fileSelectOpenCB, NULL);

	/* next row */
	filterLabel = XtVaCreateManagedWidget("filterLabel",
		labelWidgetClass, row3,
		XtNborderWidth, 0,
		XtNlabel, fw->fileSelect.filterLabel,
		XtNjustify, XtJustifyRight,
		XtNgridx, 0,
		XtNgridy, 1,
		XtNfill, FillWidth,
		0);

	fw->fileSelect.filterText = makeTextField("filterText", row3);
	XtVaSetValues(fw->fileSelect.filterText,
		XtNgridx, 1,
		XtNgridy, 1,
		XtNfill, FillWidth,
		XtNweightx, 1,
		0);

	fw->fileSelect.cancelButton = cancelButton = XtVaCreateManagedWidget("cancelButton",
		commandWidgetClass, row3,
		XtNlabel, fw->fileSelect.cancelLabel,
		XtNgridx, 2,
		XtNgridy, 1,
		XtNfill, FillWidth,
		0);
	XtAddCallback(cancelButton, XtNcallback, fileSelectCancelCB, NULL);

	/* Menus */
	fw->fileSelect.dirMenu = XtVaCreatePopupShell("dirMenu",
		simpleMenuWidgetClass, new, NULL, 0);

	addMenuItem(fw, "rootDir", "/");
	if ((ptr = getenv("HOME")) != NULL)
		addMenuItem(fw, "homeDir", ptr);

	if (getcwd(path, MAXPATHLEN) != NULL)
		addMenuItem(fw, "cwd", path);

	(void)XtCreateManagedWidget("line",
		smeLineObjectClass, fw->fileSelect.dirMenu, NULL, 0);

	fw->fileSelect.nmenu = 3;

	/* TODO: these could be cached on a per-screen basis.  */
	createbitmap(fw->fileSelect.dirButton,
		dnarrow_width, dnarrow_height, dnarrow_bits);
	createbitmap(fw->fileSelect.upButton,
		folder_up_width, folder_up_height, folder_up_bits);
	createbitmap(fw->fileSelect.newButton,
		folder_new_width, folder_new_height, folder_new_bits);
	createbitmap(fw->fileSelect.rescanButton,
		reread_width, reread_height, reread_bits);

	/* Now tie the widgets together.  Return and Escape accelerators are
	 * set for the OK and Cancel buttons respectively.
	 */

	/* Add acclerators to the open/cancel buttons to catch events
	 * elsewhere in the FileSelect widget.
	 */

	XtVaSetValues(openButton, XtNaccelerators, openAccelTbl, 0);
	XtVaSetValues(cancelButton, XtNaccelerators, cancelAccelTbl, 0);
	XtVaSetValues(new, XtNaccelerators, accelTbl, 0);

	/* Install them everywhere */
	XtInstallAccelerators(new, openButton);
	XtInstallAccelerators(new, cancelButton);
	XtInstallAccelerators(fw->fileSelect.dirButton, openButton);
	XtInstallAccelerators(fw->fileSelect.dirButton, cancelButton);
	XtInstallAccelerators(fw->fileSelect.dirName, cancelButton);
	XtInstallAccelerators(fw->fileSelect.upButton, openButton);
	XtInstallAccelerators(fw->fileSelect.upButton, cancelButton);
	XtInstallAccelerators(fw->fileSelect.newButton, openButton);
	XtInstallAccelerators(fw->fileSelect.newButton, cancelButton);
	XtInstallAccelerators(fw->fileSelect.rescanButton, openButton);
	XtInstallAccelerators(fw->fileSelect.rescanButton, cancelButton);
	XtInstallAccelerators(fw->fileSelect.fileList, openButton);
	XtInstallAccelerators(fw->fileSelect.fileList, cancelButton);
	XtInstallAccelerators(fw->fileSelect.fileName, openButton);
	XtInstallAccelerators(fw->fileSelect.fileName, cancelButton);
	XtInstallAccelerators(fw->fileSelect.filterText, cancelButton);
	XtInstallAccelerators(openButton, openButton);
	XtInstallAccelerators(openButton, cancelButton);
	XtInstallAccelerators(cancelButton, openButton);
	XtInstallAccelerators(cancelButton, cancelButton);

	/* scroll accelerators */
	XtInstallAccelerators(fw->fileSelect.dirButton, new);
	XtInstallAccelerators(fw->fileSelect.dirName, new);
	XtInstallAccelerators(fw->fileSelect.upButton, new);
	XtInstallAccelerators(fw->fileSelect.newButton, new);
	XtInstallAccelerators(fw->fileSelect.rescanButton, new);
	XtInstallAccelerators(fw->fileSelect.fileList, new);
	XtInstallAccelerators(fw->fileSelect.fileName, new);
	XtInstallAccelerators(fw->fileSelect.filterText, new);
	XtInstallAccelerators(openButton, new);
	XtInstallAccelerators(cancelButton, new);

	/* One more complication:  I want return in the directory name and filter
	 * text widgets to do something different.  If there were buttons
	 * to perform those actions, I could use accelerators, but there
	 * aren't.  It would be way cool if the Intrinsics library allowed
	 * you to bind different accelerator tables for the same widget, but
	 * it doesn't.
	 *
	 * Instead, I create a couple of global actions, __FileSelectDir()
	 * and __FileSelectFilter() and load these into the translation
	 * tables of the widgets in question.
	 */


	/* Add my actions to the global application context. */
	/* TODO: can this be done in class init? */
	XtAppAddActions(XtWidgetToApplicationContext(new),
		actionsList, XtNumber(actionsList));

		/* Add new translations to the text widgets */
	XtOverrideTranslations(fw->fileSelect.dirName, dirTransTbl);
	XtOverrideTranslations(fw->fileSelect.filterText, filterTransTbl);

	FileSelectSet(new, "./", "*", "");
}

static void freeFileNames(FileSelectWidget fw) {
	FSinfo *ptr;
	int	i;

	if ((ptr = fw->fileSelect.fileInfo) != NULL)
		for (i = fw->fileSelect.nfiles; --i >= 0; ++ptr)
			XtFree(ptr->name);

	fw->fileSelect.nfiles = 0;
}

static void FileSelectDestroy(Widget w) {
	FileSelectWidget fw = (FileSelectWidget)w;

	freeFileNames(fw);
	if (fw->fileSelect.fileNames != NULL)
		XtFree((char *)fw->fileSelect.fileNames);
	if (fw->fileSelect.fileInfo != NULL)
		XtFree((char *)fw->fileSelect.fileInfo);
}

/* Utility: call widget's parent's callback list */
static void parentCallback(Widget w, Boolean ok) {
	FileSelectWidget fw = getFsWidget(w);
	char filename[MAXPATHLEN];

	if (fw == NULL)
		return;

	if (ok)
		FileSelectGetFilename((Widget)fw, filename);

	XtCallCallbackList((Widget)fw, fw->fileSelect.callbacks,
		(XtPointer)(ok ? filename : NULL));
}

/* Handle Up button */
static void fileSelectUpCB(Widget cmd, XtPointer client, XtPointer call_data) {
	upDir(getFsWidget(cmd), call_data);
}

/* Handle New button */
static	void fileSelectNewCB(Widget	cmd, XtPointer client, XtPointer call_data) {
	/* TODO */
}

/* Handle Reread button */
static	void fileSelectRereadCB(Widget cmd, XtPointer client, XtPointer call_data) {
	fileSelectListDir(getFsWidget(cmd));
}

/* Handle Open button */
static void fileSelectOpenCB(Widget cmd, XtPointer client, XtPointer call_data) {
	parentCallback(cmd, True);
}

/* handle Cancel button */
static void fileSelectCancelCB(Widget	cmd, XtPointer	client, XtPointer	call_data) {
	parentCallback(cmd, False);
}

/* handle keystrokes in filename */
static void fileKey(Widget	txt, XtPointer client, XtPointer call_data) {
	FileSelectWidget fw = (FileSelectWidget)client;
	int i;

	if ((i = fileComplete(fw)) >= 0)
		scrollTo(fw, i, NULL);
}

/* Handle return pressed in Dir text */
/* TODO: relative paths */
static void FileSelectDir(Widget w, XEvent *event, String *params, Cardinal *num_params) {
	fileSelectListDir(getFsWidget(w));
}

/* Handle return pressed in Filter text */
static void FileSelectFilter(Widget w, XEvent *event, String *params, Cardinal *num_params) {
	fileSelectListDir(getFsWidget(w));
}

/* User pressed tab, go as far as we can */
static void FileExpand(Widget w, XEvent *event, String *params, Cardinal *num_params) {
	FileSelectWidget fw = (FileSelectWidget)w;
	FSinfo *fsi;
	int	i;

	if ((i = fileComplete(fw)) >= 0) {
		fsi = &fw->fileSelect.fileInfo[i];
		if (!S_ISDIR(fsi->mode))
			FileSelectSet((Widget)fw, NULL, NULL, fsi->name);
		else
			FileSelectSet((Widget)fw, NULL, NULL, "");
	}
}

	/* File entry scroll actions.  Slightly complicated by the fact
	 * that we want to keep the current item visible in the window.
	 * This means computing the position of the current item and
	 * scrolling as needed.
	 */
static void FileScroll(Widget w, XEvent *event, String *params, Cardinal *num_params) {
	FileSelectWidget fw = getFsWidget(w);
	Widget flist = fw->fileSelect.fileList;
	Widget fscroll = fw->fileSelect.fileScroll;
	Widget clip = XtNameToWidget(fscroll, "clip");
	XawListReturnStruct *info = XawListShowCurrent(flist);
	int	newidx;
	Dimension rowspace;
	XFontStruct *f;
	Dimension h;
	FSinfo *fsi;

	if (*num_params < 1)
		return;

	switch (**params) {
		case 'p':
	  /* height of one row */
			XtVaGetValues(flist, XtNrowSpacing, &rowspace, XtNfont, &f, 0);
			rowspace += f->ascent + f->descent;
			rowspace = max(rowspace, 1);

			/* height of entire viewport */
			if (clip)
				XtVaGetValues(clip, XtNheight, &h, 0);
			else
				h = rowspace;

			if (strcmp(*params, "pgup") == 0)
				newidx = info->list_index - h / rowspace;
			else
				newidx = info->list_index + h / rowspace;
			break;
		case 'u':
			newidx = info->list_index - 1;
			break;
		case 'd':
			newidx = info->list_index + 1;
			break;
		case 'h':
			newidx = 0;
			break;
		case 'e':
			newidx = fw->fileSelect.nfiles - 1;
			break;
	}

	newidx = max(newidx, 0);
	newidx = min(newidx, fw->fileSelect.nfiles - 1);

	fsi = &fw->fileSelect.fileInfo[newidx];
	if (!S_ISDIR(fsi->mode))
		FileSelectSet((Widget)fw, NULL, NULL, fsi->name);
	else
		FileSelectSet((Widget)fw, NULL, NULL, "");

	scrollTo(fw, newidx, clip);
}

/* Adjust viewport scroll so that item "idx" is visible */
static void scrollTo(FileSelectWidget fw, int idx, Widget clip) {
	Widget flist = fw->fileSelect.fileList;
	Dimension rowspace;
	XFontStruct *f;
	Position y, y2;
	Dimension h;

	if (clip == NULL)
		clip = XtNameToWidget(fw->fileSelect.fileScroll, "clip");

	  /* height of one row */
	XtVaGetValues(flist, XtNy, &y, XtNrowSpacing, &rowspace, XtNfont, &f, 0);
	rowspace += f->ascent + f->descent;
	rowspace = max(rowspace, 1);

	/* height of entire viewport */
	if (clip)
		XtVaGetValues(clip, XtNheight, &h, 0);
	else
		h = rowspace;

	XawListHighlight(flist, idx);

	if (XtIsRealized(fw->fileSelect.fileScroll)) {
	  /* A word of explanation:  The list widget is a child of the Viewport
	   * widget, but it's actual window is a child of a window named
	   * "clip", created by the Viewport widget.  (This is not documented
	   * anywhere that I know of, which means that this code is not portable
	   * to non-standard versions of XawM.
	   *
	   * By reading back the
	   * y-position of the list widget, we can determine the current
	   * scroll position.  By reading the height of "clip", we can determine
	   * the size of the scroll window.  We can then tell if the currently-
	   * selected item is visible.  If not, we scroll accordingly.
	   */

		y2 = rowspace * idx;

		if (y2 < -y)
			XawViewportSetCoordinates(fw->fileSelect.fileScroll, 0, y2);
		else if (y2 + rowspace > -y + h)
			XawViewportSetCoordinates(fw->fileSelect.fileScroll, 0, y2 - h + rowspace);
	}
}

	/* Handle clicks on filename */
static void fileCB(Widget w,XtPointer	client,XtPointer call_data){
	XawListReturnStruct *item = (XawListReturnStruct *)call_data;
	FileSelectWidget fw = (FileSelectWidget)client;
	Time	t = XtLastTimestampProcessed(XtDisplay(w));
	FSinfo *info;

	info = &fw->fileSelect.fileInfo[item->list_index];
	if (!S_ISDIR(info->mode))
		FileSelectSet((Widget)fw, NULL, NULL, item->string);

	if (fw->fileSelect.fileIdx == item->list_index &&
		t - fw->fileSelect.fileTime < fw->fileSelect.clickTime) {
	  /* double-click.  On dir means we change dir.  Otherwise, return
	   * to caller.
	   * TODO: caller might want a dir instead of a file.
	   */

		if (S_ISDIR(info->mode))
			appendDir(fw, item->string);
		else
			parentCallback(w, True);
	}

	fw->fileSelect.fileTime = t;
	fw->fileSelect.fileIdx = item->list_index;
}


	/* User has selected a directory from the menu */
static void menuSelectCB(Widget	w,XtPointer	client,XtPointer call_data){
	String	dir;

	XtVaGetValues(w, XtNlabel, &dir, 0);
	FileSelectSet((Widget)getFsWidget(w), dir, NULL, NULL);
}

	/* head up tree until we find the parent */
static	FileSelectWidget getFsWidget(Widget	w){
	while (w != NULL && XtClass(w) != fileSelectWidgetClass)
		w = XtParent(w);
	return (FileSelectWidget)w;
}

static int fileCmp(const void *aa,const void *bb){
	FSinfo *a = (FSinfo *)aa;
	FSinfo *b = (FSinfo *)bb;

	if (S_ISDIR(a->mode) == S_ISDIR(b->mode))
		return strcmp(a->name, b->name);

	if (S_ISDIR(a->mode)) return -1;

	return 1;
}

	/* list the directory in path and place in file list */
static void fileSelectListDir(FileSelectWidget	fw) {
	DIR *idir;
	struct dirent *dirent;
	struct stat	statbuf;
	String	path;
	char	pathname[MAXPATHLEN], *pathend;
	char	name[MAXPATHLEN];
	char *filters[20];
	int		nf;
	char *ptr;
	int		nfiles = 0;
	int		i;

	if (fw->fileSelect.fileNames == NULL) {
		fw->fileSelect.fileNames = MALLOC(String, fw->fileSelect.nfalloc = 16);
		fw->fileSelect.fileInfo = MALLOC(FSinfo, fw->fileSelect.nfalloc);
	}

	fw->fileSelect.nfiles = 0;

	XtVaGetValues(fw->fileSelect.dirName, XtNstring, &path, 0);
	strncpy(pathname, path, XtNumber(pathname));

	XtVaGetValues(fw->fileSelect.filterText, XtNstring, &filters[0], 0);
	if (strlen(filters[0]) == 0) filters[0] = "*";
	filters[0] = XtNewString(filters[0]);
	ptr = strtok(filters[0], " \t,");
	for (nf = 0; nf < XtNumber(filters) && ptr != NULL; ) {
		filters[nf++] = ptr;
		ptr = strtok(NULL, " \t,");
	}

	while ((idir = opendir(pathname)) == NULL &&
		errno == ENOTDIR &&
		(ptr = strrchr(pathname, '/')) != NULL)
		*ptr = '\0';

	if (idir != NULL)		/* TODO: error message? */
	{
		freeFileNames(fw);
		strcat(pathname, "/");
		pathend = pathname + strlen(pathname);
		while ((dirent = readdir(idir)) != NULL) {
			strcpy(pathend, dirent->d_name);
			i = stat(pathname, &statbuf);

			/* TODO: better handling of directories.  Icons would be
			 * nice.
			 */

			if (i >= 0 && strcmp(dirent->d_name, ".") != 0) {
				if (S_ISDIR(statbuf.st_mode) ||
					fileSelectMatchFile2(dirent->d_name, filters, nf)) {
					if (nfiles + 1 >= fw->fileSelect.nfalloc) {
						fw->fileSelect.nfalloc *= 2;
						REALLOC(fw->fileSelect.fileNames, String, fw->fileSelect.nfalloc);
						REALLOC(fw->fileSelect.fileInfo, FSinfo, fw->fileSelect.nfalloc);
					}
					if (S_ISDIR(statbuf.st_mode)) {
						strcpy(name, dirent->d_name);
						strcat(name, "/");
						ptr = name;
					} else
						ptr = dirent->d_name;
					fw->fileSelect.fileInfo[nfiles].name = XtNewString(ptr);
					fw->fileSelect.fileInfo[nfiles].mode = statbuf.st_mode;
					++nfiles;
				}
			}
		}

		XtFree(filters[0]);

		fw->fileSelect.nfiles = nfiles;
		qsort((void *)fw->fileSelect.fileInfo, fw->fileSelect.nfiles,
			sizeof(FSinfo), fileCmp);

		for (i = 0; i < nfiles; ++i)
			fw->fileSelect.fileNames[i] = fw->fileSelect.fileInfo[i].name;

		(void)closedir(idir);
	}

	fw->fileSelect.fileNames[nfiles] = NULL;

	XawListChange(fw->fileSelect.fileList, (String *)fw->fileSelect.fileNames, nfiles, 0, True);

	if (XtIsRealized(fw->fileSelect.fileScroll))
		XawViewportSetCoordinates(fw->fileSelect.fileScroll, 0, 0);
}

	/* quick-and-dirty recursive pattern matcher for shell-style
	 * regular expressions
	 */
static Boolean fileSelectMatchFile2(String	file, String *pattern, int	np) {
	while (--np >= 0)
		if (fileSelectMatchFile(file, *pattern++))
			return True;
	return False;
}

static Boolean fileSelectMatchFile(String file, String pattern) {
	Boolean	rval;
	for (;;) {
		switch (*pattern) {
			case '\0':			/* end of pattern match end of name */
				return *file == '\0';

			default:
				if (*pattern != *file)	/* simple character match */
					return False;
				++pattern;
				++file;
				break;

			case '?':			/* '?' match anything but nul */
				if (*file == '\0')
					return False;
				++pattern;
				++file;
				break;

			case '*':
				if (*++pattern == '\0')	/* trailing '*' match anything */
					return True;
				for (; *file != '\0'; ++file)	/* else match rest of pat */
					if (fileSelectMatchFile(file, pattern))
						return True;
				return False;

			case '[':			/* [...] match anything in list */
				rval = False;
				for (++pattern;
					*pattern != ']' && *pattern != '\0';
					++pattern)
					if (!rval) {
						if (pattern[1] == '-') {
							rval = *file >= pattern[0] && *file <= pattern[2];
							++pattern;
						} else
							rval = *file == pattern[0];
					}
				if (!rval)
					return False;
				if (*pattern == ']') ++pattern;
				++file;
				break;
		}
	}
}

	/* Append the given name to the current directory.  Make
	 * sure that '/' occurs in all the right places.  If
	 * dir is "../", then execute upDir instead.
	 */
static void appendDir(FileSelectWidget fw, String	dir) {
	char	pathname[MAXPATHLEN], *ptr;
	int	plen, len;

	if (strcmp(dir, "..") == 0 || strcmp(dir, "../") == 0) {
		upDir(fw, dir);
		return;
	}

	XtVaGetValues(fw->fileSelect.dirName, XtNstring, &ptr, 0);
	plen = strlen(ptr);
	len = strlen(dir);
	if (plen + len + 4 < MAXPATHLEN) {
		memcpy(pathname, ptr, plen);
		if (pathname[plen - 1] != '/')
			strcpy(pathname + plen++, "/");
		strcpy(pathname + plen, dir); plen += len;
		if (pathname[plen - 1] != '/')
			strcpy(pathname + plen++, "/");
		FileSelectSet((Widget)fw, pathname, NULL, NULL);
	}
}

/* Move up one directory in the chain. */
static void upDir(FileSelectWidget fw, String dir) {
	char	pathname[MAXPATHLEN], *ptr;
	int	plen;

	XtVaGetValues(fw->fileSelect.dirName, XtNstring, &ptr, 0);
	plen = strlen(ptr);
	if (plen == 1 && *ptr == '/')
		return;

	if (plen == 0 || (plen == 2 && strcmp(ptr, "./") == 0)) {
		(void)getcwd(pathname, MAXPATHLEN);
		plen = strlen(pathname);
	} else
		memcpy(pathname, ptr, plen);

	if (pathname[plen - 1] == '/')
		pathname[--plen] = '\0';
	if ((ptr = strrchr(pathname, '/')) != NULL)
		ptr[1] = '\0';

	FileSelectSet((Widget)fw, pathname, NULL, NULL);
}

/* expand filename as far as we can, return relevant index */
static	int fileComplete(FileSelectWidget fw) {
	String s;
	int len, i, j;
	FSinfo *fsi = fw->fileSelect.fileInfo;

	XtVaGetValues(fw->fileSelect.fileName, XtNstring, &s, 0);
	if ((len = strlen(s)) <= 0)
		return -1;

	for (i = 0; i < fw->fileSelect.nfiles; ++i, ++fsi)
		if ((j = strncmp(s, fsi->name, len)) == 0)
			return i;

	return -1;
}

static Widget makeTextField(char *name, Widget parent) {
	return XtVaCreateManagedWidget(name,
		asciiTextWidgetClass, parent,
		XtNresizable, True,
		XtNresize, XawtextResizeWidth,
		XtNeditType, XawtextEdit,
		0);
}

static void addMenuItem(FileSelectWidget fw, char *name, char *value) {
	Widget item;
	char path[MAXPATHLEN];
	int i = strlen(value);

	if (value[i - 1] != '/') {
		strcpy(path, value);
		strcpy(path + i, "/");
		value = path;
	}

	item = XtVaCreateManagedWidget(name,
		smeBSBObjectClass, fw->fileSelect.dirMenu,
		XtNlabel, value,
		0);
	XtAddCallback(item, XtNcallback, menuSelectCB, NULL);
}

/* add item to menu if there's room and if it's not already there */
static void addMenuMaybe(FileSelectWidget fw, String dir) {
	Widget *childP;
	CompositeWidget menu;
	String	value;
	int	i;

	if (fw->fileSelect.nmenu > MAX_MENU)
		return;
	else
		++fw->fileSelect.nmenu;

	  /* Iterate over menu children, look for one with the same value
	   * as dir.  If found, then do nothing.
	   * TODO: is there a more legitimate way to do this?
	   */

	menu = (CompositeWidget)fw->fileSelect.dirMenu;
	childP = menu->composite.children;
	for (i = menu->composite.num_children; --i >= 0; ++childP) {
		if (XtIsSubclass(*childP, smeBSBObjectClass)) {
			XtVaGetValues(*childP, XtNlabel, &value, 0);
			if (strcmp(value, dir) == 0)
				return;
		}
	}

	addMenuItem(fw, "dirItem", dir);
}

/* if widget does not yet have a bitmap, create one now. */
static void createbitmap(Widget	w, int wid, int hgt, unsigned char *bits) {
	Pixmap	bm;

	XtVaGetValues(w, XtNbitmap, &bm, 0);
	if (bm == None) {
		Window root = RootWindowOfScreen(XtScreen(w));
		bm = XCreateBitmapFromData(XtDisplay(w), root,
			(char *)bits, wid, hgt);
		XtVaSetValues(w, XtNbitmap, bm, 0);
	}
}

/* Read current full path */
void FileSelectGetFilename(Widget w, String path) {
	FileSelectWidget fw = (FileSelectWidget)w;
	String value;
	int i;

	XtVaGetValues(fw->fileSelect.dirName, XtNstring, &value, 0);
	i = strlen(value);
	memcpy(path, value, i + 1);
	if (path[i - 1] != '/') strcpy(path + i, "/");
	XtVaGetValues(fw->fileSelect.fileName, XtNstring, &value, 0);
	strcat(path + i, value);
}

	/* Set directory, filter and/or filename.  If directory is
	 * NULL and filename is a full path, does the right thing.
	 */
void FileSelectSet(Widget w, String dir, String filter, String file) {
	FileSelectWidget fw = (FileSelectWidget)w;
	char	pathname[MAXPATHLEN], *ptr;
	char	newPath[MAXPATHLEN];
	int		i;

	if (dir == NULL &&
		file != NULL && (ptr = strrchr(file, '/')) != NULL) {
		  /* user provided a full path; split it up */
		strncpy(newPath, file, MAXPATHLEN);
		file = strrchr(newPath, '/');
		*file++ = '\0';
		dir = newPath;
	}

	if (dir != NULL) {
		addMenuMaybe(fw, dir);
		i = strlen(dir);
		if (dir[i - 1] != '/') {
			memcpy(pathname, dir, i);
			strcpy(pathname + i, "/");
			dir = pathname;
		}
		XtVaSetValues(fw->fileSelect.dirName, XtNstring, dir, 0);
	}

	if (filter != NULL)
		XtVaSetValues(fw->fileSelect.filterText, XtNstring, filter, 0);

	if (file != NULL)
		XtVaSetValues(fw->fileSelect.fileName, XtNstring, file, 0);

	if (dir != NULL || filter != NULL)
		fileSelectListDir(fw);
}

Widget FileSelectCreateWindow(String name, Widget parent,
	XtCallbackProc cb, XtPointer client) {
	Widget	shell;
	Widget	fw;
	char	winName[256];

	strncpy(winName, name, XtNumber(winName) - 8);
	strcat(winName, "_win");
	shell = XtVaCreatePopupShell(winName,
		transientShellWidgetClass, parent,
		XtNtransientFor, parent,
		0);

	fw = XtVaCreateManagedWidget(name, fileSelectWidgetClass, shell, 0);
	if (cb != NULL)
		XtAddCallback(fw, XtNcallback, cb, client);

	return fw;
}

void FileSelectDestroyWindow(Widget fw) {
	XtDestroyWidget(XtParent(fw));
}

void FileSelectPopup(Widget fw) {
	XtPopup(XtParent(fw), XtGrabNone);
}

void FileSelectPopdown(Widget fw) {
	XtPopdown(XtParent(fw));
}

Widget FileSelectParent(Widget fw) {
	return XtParent(fw);
}
