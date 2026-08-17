/* $Id: XawAlloc.h,v 1.1 1999/12/15 19:12:02 falk Exp $ */

#define	XtStackAlloc(size, stack_cache_array)	\
    ((size) <= sizeof(stack_cache_array)	\
    ? (XtPointer)(stack_cache_array)		\
    : XtMalloc((unsigned)(size)))

#define	XtStackFree(pointer, stack_cache_array)	\
    if ((pointer) != ((XtPointer)(stack_cache_array))) XtFree(pointer); else

