/*
 * $Id: FileSelectP.h,v 1.2 1999/03/01 08:58:23 falk Exp $
 *
 * Written by Edward A. Falk (falk@falconer.vip.best.com)
 * Loosely based on (but not subclassed from) Dialog
 */


/* Private definitions for FileSelect widget */

#ifndef _FileSelectP_h
#define _FileSelectP_h

#include <Linux/FileSelect.h>
#include "GridboxP.h"

typedef struct {XtPointer extension;} FileSelectClassPart;

typedef struct _FileSelectClassRec {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    GridboxClassPart	gridbox_class;
    FileSelectClassPart	fileSelect_class;
} FileSelectClassRec;

extern FileSelectClassRec fileSelectClassRec;

typedef struct _FileSelectPart {
    /* resources */
    String	filterLabel ;
    String	dirLabel ;
    String	fileLabel ;
    String	selectionLabel ;
    String	openLabel ;
    String	cancelLabel ;
    XtCallbackList callbacks ;	/* when user hits Open or Cancel */
    int		clickTime ;

    /* private data */
    Widget	dirName ;	/* current directory */
    Widget	dirButton ;	/* directory menu button */
    Widget	upButton ;	/* up one dir */
    Widget	newButton ;	/* create dir */
    Widget	rescanButton ;	/* reread dir */
    Widget	fileList;	/* list of files		*/
    Widget	fileName;	/* user response TextWidget	*/
    Widget	filterText;	/* widget to display dir/filter	*/
    Widget	openButton ;
    Widget	cancelButton ;
    Widget	fileScroll, scrollbar ;
    Widget	dirMenu ;
    int		nmenu ;
    struct fsinfo *fileInfo ;	/* file info */
    String	*fileNames ;	/* file names for list widget */
    int		nfiles, nfalloc ;
    Time	fileTime ;
    int		fileIdx ;
    XtPointer	extension ;
} FileSelectPart;

typedef struct _FileSelectRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    GridboxPart		gridbox;
    FileSelectPart	fileSelect;
} FileSelectRec;

typedef struct {XtPointer extension;} FileSelectConstraintsPart;

typedef struct _FileSelectConstraintsRec {
    GridboxConstraintsPart	gridbox;
    FileSelectConstraintsPart	fileSelect;
} FileSelectConstraintsRec, *FileSelectConstraints;

#endif /* _FileSelectP_h */
