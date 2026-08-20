#ifndef _OBJC_COCOA_H
#define _OBJC_COCOA_H

#ifndef C_API
#	define C_API extern
#endif

#if defined(__APPLE__)
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFBase.h>
#include <NSSystemDirectories.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreGraphics/CGGeometry.h>
#include <objc/NSObjCRuntime.h>
#include <objc/objc-runtime.h>
#include <objc/objc.h>
#include <limits.h>

struct webview_priv {
	id pool;
	id window;
	id webview;
	id windowDelegate;
	int should_exit;
};

typedef enum {
	NSApplicationActivationPolicyRegular = 0,
	NSApplicationActivationPolicyAccessory = 1,
	NSApplicationActivationPolicyERROR = 2,
} NSApplicationActivationPolicy;

typedef enum {
	NSWindowStyleMaskBorderless = 0,
	NSWindowStyleMaskTitled = 1 << 0,
	NSWindowStyleMaskClosable = 1 << 1,
	NSWindowStyleMaskMiniaturizable = 1 << 2,
	NSWindowStyleMaskResizable = 1 << 3,
	NSWindowStyleMaskUnifiedTitleAndToolbar = 1 << 12,
} NSWindowStyleMask;

typedef enum {
	NSBackingStoreRetained = 0,
	NSBackingStoreNonretained = 1,
	NSBackingStoreBuffered = 2,
} NSBackingStoreType;

/* various types of events */
typedef enum {
	NSEventTypeLeftMouseDown = 1,
	NSEventTypeLeftMouseUp = 2,
	NSEventTypeRightMouseDown = 3,
	NSEventTypeRightMouseUp = 4,
	NSEventTypeMouseMoved = 5,
	NSEventTypeLeftMouseDragged = 6,
	NSEventTypeRightMouseDragged = 7,
	NSEventTypeMouseEntered = 8,
	NSEventTypeMouseExited = 9,
	NSEventTypeKeyDown = 10,
	NSEventTypeKeyUp = 11,
	NSEventTypeFlagsChanged = 12,
	NSEventTypeAppKitDefined = 13,
	NSEventTypeSystemDefined = 14,
	NSEventTypeApplicationDefined = 15,
	NSEventTypePeriodic = 16,
	NSEventTypeCursorUpdate = 17,
	NSEventTypeScrollWheel = 22,
	NSEventTypeTabletPoint = 23,
	NSEventTypeTabletProximity = 24,
	NSEventTypeOtherMouseDown = 25,
	NSEventTypeOtherMouseUp = 26,
	NSEventTypeOtherMouseDragged = 27,
} NSEventType;

typedef enum {
	NSAlertStyleWarning, /* An alert style to warn someone about a current or impending event. */
	NSAlertStyleInformational, /* An alert style to inform someone about a current or impending event. */
	NSAlertStyleCritical, /* An alert style to inform someone about a critical event. */
} NSAlertStyle;

enum {
	NSAlertFirstButtonReturn = 1000,
	NSAlertSecondButtonReturn = 1001,
	NSAlertThirdButtonReturn = 1002,
};

typedef enum NSEventMask { /* masks for the types of events */
	NSEventMaskLeftMouseDown = 1ULL << NSEventTypeLeftMouseDown,
	NSEventMaskLeftMouseUp = 1ULL << NSEventTypeLeftMouseUp,
	NSEventMaskRightMouseDown = 1ULL << NSEventTypeRightMouseDown,
	NSEventMaskRightMouseUp = 1ULL << NSEventTypeRightMouseUp,
	NSEventMaskMouseMoved = 1ULL << NSEventTypeMouseMoved,
	NSEventMaskLeftMouseDragged = 1ULL << NSEventTypeLeftMouseDragged,
	NSEventMaskRightMouseDragged = 1ULL << NSEventTypeRightMouseDragged,
	NSEventMaskMouseEntered = 1ULL << NSEventTypeMouseEntered,
	NSEventMaskMouseExited = 1ULL << NSEventTypeMouseExited,
	NSEventMaskKeyDown = 1ULL << NSEventTypeKeyDown,
	NSEventMaskKeyUp = 1ULL << NSEventTypeKeyUp,
	NSEventMaskFlagsChanged = 1ULL << NSEventTypeFlagsChanged,
	NSEventMaskAppKitDefined = 1ULL << NSEventTypeAppKitDefined,
	NSEventMaskSystemDefined = 1ULL << NSEventTypeSystemDefined,
	NSEventMaskApplicationDefined = 1ULL << NSEventTypeApplicationDefined,
	NSEventMaskPeriodic = 1ULL << NSEventTypePeriodic,
	NSEventMaskCursorUpdate = 1ULL << NSEventTypeCursorUpdate,
	NSEventMaskScrollWheel = 1ULL << NSEventTypeScrollWheel,
	NSEventMaskTabletPoint = 1ULL << NSEventTypeTabletPoint,
	NSEventMaskTabletProximity = 1ULL << NSEventTypeTabletProximity,
	NSEventMaskOtherMouseDown = 1ULL << NSEventTypeOtherMouseDown,
	NSEventMaskOtherMouseUp = 1ULL << NSEventTypeOtherMouseUp,
	NSEventMaskOtherMouseDragged = 1ULL << NSEventTypeOtherMouseDragged,
} NSEventMask;

enum {
	NSViewNotSizable = 0x00,
	NSViewMinXMargin = 0x01,
	NSViewWidthSizable = 0x02,
	NSViewMaxXMargin = 0x04,
	NSViewMinYMargin = 0x08,
	NSViewHeightSizable = 0x10,
	NSViewMaxYMargin = 0x20
};

typedef enum {
	NSNoBorder,
	NSLineBorder,
	NSBezelBorder,
	NSGrooveBorder
} NSBorderType;

enum {
	NSViewLayerContentsPlacementScaleAxesIndependently = 0,
	NSViewLayerContentsPlacementScaleProportionallyToFit,
	NSViewLayerContentsPlacementScaleProportionallyToFill,
	NSViewLayerContentsPlacementCenter,
	NSViewLayerContentsPlacementTop,
	NSViewLayerContentsPlacementTopRight,
	NSViewLayerContentsPlacementRight,
	NSViewLayerContentsPlacementBottomRight,
	NSViewLayerContentsPlacementBottom,
	NSViewLayerContentsPlacementBottomLeft,
	NSViewLayerContentsPlacementLeft,
	NSViewLayerContentsPlacementTopLeft
};
typedef NSInteger NSViewLayerContentsPlacement;

enum {
	NSViewLayerContentsRedrawNever = 0,
	NSViewLayerContentsRedrawOnSetNeedsDisplay,
	NSViewLayerContentsRedrawDuringViewResize,
	NSViewLayerContentsRedrawBeforeViewResize
};

typedef unsigned NSFontTraitMask;
typedef enum {
	NSNoFontChangeAction = 0,
	NSViaPanelFontAction = 1,
	NSAddTraitFontAction = 2,
	NSSizeUpFontAction = 3,
	NSSizeDownFontAction = 4,
	NSHeavierFontAction = 5,
	NSLighterFontAction = 6,
	NSRemoveTraitFontAction = 7,
} NSFontAction;

enum {
	NSItalicFontMask = 0x00000001,
	NSBoldFontMask = 0x00000002,
	NSUnboldFontMask = 0x00000004,
	NSNonStandardCharacterSetFontMask = 0x00000008,
	NSNarrowFontMask = 0x00000010,
	NSExpandedFontMask = 0x00000020,
	NSCondensedFontMask = 0x00000040,
	NSSmallCapsFontMask = 0x00000080,
	NSPosterFontMask = 0x00000100,
	NSCompressedFontMask = 0x00000200,
	NSFixedPitchFontMask = 0x00000400,
	NSUnitalicFontMask = 0x01000000,
};

enum {
	NSNoCellMask = 0x00,
	NSContentsCellMask = 0x01,
	NSPushInCellMask = 0x02,
	NSChangeGrayCellMask = 0x04,
	NSChangeBackgroundCellMask = 0x08,
};

typedef enum {
	NSMomentaryLightButton = 0,
	NSPushOnPushOffButton = 1,
	NSToggleButton = 2,
	NSSwitchButton = 3,
	NSRadioButton = 4,
	NSMomentaryChangeButton = 5,
	NSOnOffButton = 6,
	NSMomentaryPushInButton = 7,
	// deprecated values
	NSMomentaryPushButton = 0,
	NSMomentaryLight = 7
} NSButtonType;

typedef enum {
	NSBezelStyleRounded = 1,
	NSBezelStyleRegularSquare = 2,
	NSBezelStyleDisclosure = 5,
	NSBezelStyleShadowlessSquare = 6,
	NSBezelStyleCircular = 7,
	NSBezelStyleTexturedSquare = 8,
	NSBezelStyleHelpButton = 9,
	NSBezelStyleSmallSquare = 10,
	NSBezelStyleTexturedRounded = 11,
	NSBezelStyleRoundRect = 12,
	NSBezelStyleRecessed = 13,
	NSBezelStyleRoundedDisclosure = 14,
	NSBezelStyleInline = 15,
} _NSBezelStyle;

typedef enum {
	NSRoundedBezelStyle = 1,
	NSRegularSquareBezelStyle = 2,
	NSThickSquareBezelStyle = 3,
	NSThickerSquareBezelStyle = 4,
	NSDisclosureBezelStyle = 5,
	NSShadowlessSquareBezelStyle = 6,
	NSCircularBezelStyle = 7,
	NSTexturedSquareBezelStyle = 8,
	NSHelpButtonBezelStyle = 9,
	NSSmallSquareBezelStyle = 10,
	NSTexturedRoundedBezelStyle = 11,
	NSRoundRectBezelStyle = 12,
	NSRecessedBezelStyle = 13,
	NSRoundedDisclosureBezelStyle = 14,
} NSBezelStyle;

typedef enum {
	NSGradientNone = 0,
	NSGradientConcaveWeak = 1,
	NSGradientConcaveStrong = 2,
	NSGradientConvexWeak = 3,
	NSGradientConvexStrong = 4,
} NSGradientType;

typedef uint16_t unichar;
typedef enum {
	NSASCIIStringEncoding = 1,
	NSNEXTSTEPStringEncoding = 2,
	NSJapaneseEUCStringEncoding = 3,
	NSUTF8StringEncoding = 4,
	NSISOLatin1StringEncoding = 5,
	NSSymbolStringEncoding = 6,
	NSNonLossyASCIIStringEncoding = 7,
	NSShiftJISStringEncoding = 8,
	NSISOLatin2StringEncoding = 9,
	NSUnicodeStringEncoding = 10,
	NSWindowsCP1251StringEncoding = 11,
	NSWindowsCP1252StringEncoding = 12,
	NSWindowsCP1253StringEncoding = 13,
	NSWindowsCP1254StringEncoding = 14,
	NSWindowsCP1250StringEncoding = 15,
	NSISO2022JPStringEncoding = 21,
	NSMacOSRomanStringEncoding = 30,
	NSProprietaryStringEncoding = 0x00010000,
	NSUTF16BigEndianStringEncoding = 0x90000100,
	NSUTF16LittleEndianStringEncoding = 0x94000100,
	NSUTF32StringEncoding = 0x8c000100,
	NSUTF32BigEndianStringEncoding = 0x98000100,
	NSUTF32LittleEndianStringEncoding = 0x9c000100,
} NSStringEncoding;

enum {
	NSCaseInsensitiveSearch = 0x01,
	NSLiteralSearch = 0x02,
	NSBackwardsSearch = 0x04,
	NSAnchoredSearch = 0x08,
	NSNumericSearch = 0x40,
};

enum {
	NSStringEncodingConversionAllowLossy = 1,
	NSStringEncodingConversionExternalRepresentation = 2
};

enum {
	NSSpellingStateSpellingFlag = 0x01,
	NSSpellingStateGrammarFlag = 0x02,
};

enum {
	NSUnderlineStyleNone,
	NSUnderlineStyleSingle,
	NSUnderlineStyleThick,
	NSUnderlineStyleDouble,
};

// Deprecated constants
enum {
	NSNoUnderlineStyle = NSUnderlineStyleNone,
	NSSingleUnderlineStyle = NSUnderlineStyleSingle,
};

enum {
	NSUnderlinePatternSolid = 0x000,
	NSUnderlinePatternDot = 0x100,
	NSUnderlinePatternDash = 0x200,
	NSUnderlinePatternDashDot = 0x300,
	NSUnderlinePatternDashDotDot = 0x400,
};

typedef enum {
	NSControlStateValueMixed = -1,
	NSControlStateValueOff = 0,
	NSControlStateValueOn = 1,
} NSControlStateValue;

typedef struct AppDel {
	Class isa;

	// Will be an NSWindow later.
	id window;
} AppDelegate;

typedef NSUInteger NSStringCompareOptions;
typedef NSUInteger NSStringEncodingConversionOptions;

typedef double NSTimeInterval;
typedef CFRange NSRange;
typedef NSRange *NSRangePointer;
typedef CGSize NSSize;
typedef NSInteger NSViewLayerContentsRedrawPolicy;
typedef NSInteger NSModalResponse;
typedef CGPoint NSPoint;
typedef CGRect NSRect;
typedef CGFloat NSFloat;
typedef CGColorRef NSColor;
typedef Class NSString;
typedef Class NSAttributedString;
typedef Class NSArray;
typedef NSArray NSMutableArray;
typedef Class NSDictionary;
typedef Class NSURL;
typedef Class NSFont;
typedef Class NSFontFamily;
typedef id NSTextView;
typedef id NSWindow;
typedef id NSScrollView;
typedef id NSTextField;
typedef id NSButton;
typedef id NSMenu;
typedef id NSMenuItem;
typedef id NSEvent;
typedef id NSAlert;
typedef id NSView;
typedef id NSSavePanel;
typedef id NSOpenPanel;
typedef id NSData;
typedef id NSPredicate;
typedef Class WKWebViewConfiguration;
typedef Class WKWebView;
typedef id NSURLRequest;

#define NSNotFound NSIntegerMax
#define NSVariableStatusItemLength (-1)
#define NSSquareStatusItemLength (-2)

typedef id(*cocoa_window_cb)(id, SEL, NSRect, int, int, bool);
typedef id(*cocoa_sendrect_cb)(id, SEL, NSRect);
typedef id(*cocoa_sendview_cb)(id, SEL, NSRect, id);
typedef id(*cocoa_menu_cb)(id, SEL, NSString *title, SEL action, NSString *key);
typedef id(*cocoa_event_cb)(id, SEL, unsigned long mask, id expiration, id mode, BOOL deqFlag);
typedef id(*cocoa_send_cb)(id, SEL);
typedef id(*cocoa_sendclass_cb)(id, SEL, id);
typedef id(*cocoa_sendint_cb)(id, SEL, NSInteger);
typedef id(*cocoa_sendvariadic_cb)(id, SEL, id, va_list);
typedef id(*cocoa_sendpair_cb)(id, SEL, id, id);
typedef id(*cocoa_sendwithpair_cb)(id, SEL, id, id, NSInteger);
typedef id(*cocoa_sendwith_cb)(id, SEL, id *, NSInteger);
typedef id(*cocoa_sendfloat_cb)(id, SEL, id, NSFloat);
typedef id(*cocoa_sendany_cb)(id, SEL, void *);
typedef NSInteger(*cocoa_intpair_cb)(id, SEL, id, id);
typedef NSInteger(*cocoa_int_cb)(id, SEL);
typedef NSInteger(*cocoa_sendwithint_cb)(id, SEL, id, NSInteger);
typedef NSInteger(*cocoa_intwith_cb)(id, SEL, id);
typedef NSSize(*cocoa_size_cb)(id, SEL);
typedef NSRect *(*cocoa_rect_cb)(id, SEL);
typedef NSRange(*cocoa_range_cb)(id, SEL, id, NSInteger);

typedef void(^IMP_INT)(NSInteger);
typedef void(*cocoa_modelint_cb)(id, SEL, id, IMP_INT);
typedef void(*cocoa_post_cb)(id, SEL);
typedef void(*cocoa_postpoint_cb)(id, SEL, CGPoint);
typedef void(*cocoa_postrect_cb)(id, SEL, CGRect);
typedef void(*cocoa_postrectint_cb)(id, SEL, CGRect *, NSInteger);
typedef void(*cocoa_postsize_cb)(id, SEL, CGSize);
typedef void(*cocoa_postint_cb)(id, SEL, NSInteger);
typedef void(*cocoa_model_cb)(id, SEL, id, IMP);
typedef void(*cocoa_postid_cb)(id, SEL, id);
typedef void(*cocoa_postpair_cb)(id, SEL, id, id);
typedef void(*cocoa_postpairwith_cb)(id, SEL, id, NSInteger, id);
typedef void(*cocoa_postany_cb)(id, SEL, void *);
typedef void(*cocoa_postfunc_cb)(id, SEL, SEL);
typedef void(*cocoa_postnotification_cb)(id, SEL, id, SEL, id, id);

C_API const NSUInteger NSMaximumStringLength;
C_API id const NSDefaultRunLoopMode;
C_API id const NSEventTrackingRunLoopMode;
C_API id const NSApp;
C_API NSString const NSWindowDidBecomeKeyNotification;
C_API NSString const NSWindowDidResignKeyNotification;
C_API NSString const NSWindowDidBecomeMainNotification;
C_API NSString const NSWindowDidResignMainNotification;
C_API NSString const NSWindowWillMiniaturizeNotification;
C_API NSString const NSWindowDidMiniaturizeNotification;
C_API NSString const NSWindowDidDeminiaturizeNotification;
C_API NSString const NSWindowWillMoveNotification;
C_API NSString const NSWindowDidMoveNotification;
C_API NSString const NSWindowDidResizeNotification;
C_API NSString const NSWindowDidUpdateNotification;
C_API NSString const NSWindowWillCloseNotification;
C_API NSString const NSWindowWillStartLiveResizeNotification;
C_API NSString const NSWindowDidEndLiveResizeNotification;
C_API NSArray NSSearchPathForDirectoriesInDomains(NSSearchPathDirectory directory,
	NSSearchPathDomainMask domainMask, BOOL expandTilde);
C_API NSRange NSMakeRange(NSUInteger location, NSUInteger length);
C_API BOOL NSEqualRanges(NSRange range, NSRange otherRange);
C_API NSUInteger NSMaxRange(NSRange range);
C_API NSString NSStringFromRange(NSRange range);
C_API NSRange NSRangeFromString(NSString *s);

C_API BOOL NSLocationInRange(NSUInteger location, NSRange range);
C_API NSRange NSIntersectionRange(NSRange range, NSRange otherRange);
C_API NSRange NSUnionRange(NSRange range, NSRange otherRange);

C_API NSString const NSFontAttributeName;
C_API NSString const NSParagraphStyleAttributeName;
C_API NSString const NSForegroundColorAttributeName;
C_API NSString const NSBackgroundColorAttributeName;
C_API NSString const NSUnderlineStyleAttributeName;
C_API NSString const NSUnderlineColorAttributeName;
C_API NSString const NSAttachmentAttributeName;
C_API NSString const NSKernAttributeName;
C_API NSString const NSLigatureAttributeName;
C_API NSString const NSStrikethroughStyleAttributeName;
C_API NSString const NSStrikethroughColorAttributeName;
C_API NSString const NSObliquenessAttributeName;
C_API NSString const NSStrokeWidthAttributeName;
C_API NSString const NSStrokeColorAttributeName;
C_API NSString const NSBaselineOffsetAttributeName;
C_API NSString const NSSuperscriptAttributeName;
C_API NSString const NSLinkAttributeName;
C_API NSString const NSShadowAttributeName;
C_API NSString const NSExpansionAttributeName;
C_API NSString const NSCursorAttributeName;
C_API NSString const NSToolTipAttributeName;
C_API NSString const NSBackgroundColorDocumentAttribute;
C_API NSString const NSSpellingStateAttributeName;

C_API cocoa_window_cb cocoa_window_func;
C_API cocoa_rect_cb cocoa_rect_func;
C_API cocoa_send_cb cocoa_send_func;
C_API cocoa_sendclass_cb cocoa_sendclass_func;
C_API cocoa_sendrect_cb cocoa_sendrect_func;
C_API cocoa_sendview_cb cocoa_sendview_func;
C_API cocoa_sendany_cb cocoa_sendany_func;
C_API cocoa_sendwith_cb cocoa_sendwith_func;
C_API cocoa_sendfloat_cb cocoa_sendfloat_func;
C_API cocoa_sendvariadic_cb cocoa_sendvariadic_func;
C_API cocoa_sendpair_cb cocoa_sendpair_func;
C_API cocoa_sendwithpair_cb cocoa_sendwithpair_func;
C_API cocoa_sendint_cb cocoa_sendint_func;
C_API cocoa_event_cb cocoa_event_func;
C_API cocoa_menu_cb cocoa_menu_func;
C_API cocoa_sendwithint_cb cocoa_sendwithint_func;
C_API cocoa_intwith_cb cocoa_intwith_func;
C_API cocoa_range_cb cocoa_range_func;

C_API cocoa_post_cb cocoa_post_func;
C_API cocoa_postpoint_cb cocoa_postpoint_func;
C_API cocoa_postsize_cb cocoa_postsize_func;
C_API cocoa_postrect_cb cocoa_postrect_func;
C_API cocoa_postrectint_cb cocoa_postrectint_func;
C_API cocoa_postint_cb cocoa_postint_func;
C_API cocoa_postany_cb cocoa_postany_func;
C_API cocoa_postfunc_cb cocoa_postfunc_func;
C_API cocoa_postnotification_cb cocoa_postnotification_func;
C_API cocoa_postpair_cb cocoa_postpair_func;
C_API cocoa_postpairwith_cb cocoa_postpairwith_func;
C_API cocoa_postid_cb cocoa_postid_func;
C_API cocoa_model_cb cocoa_model_func;
C_API cocoa_modelint_cb cocoa_modelint_func;
C_API cocoa_intpair_cb cocoa_intpair_func;
C_API cocoa_int_cb cocoa_int_func;
C_API cocoa_size_cb cocoa_size_func;

typedef enum {
	NSCompositeClear,
	NSCompositeCopy,
	NSCompositeSourceOver,
	NSCompositeSourceIn,
	NSCompositeSourceOut,
	NSCompositeSourceAtop,
	NSCompositeDestinationOver,
	NSCompositeDestinationIn,
	NSCompositeDestinationOut,
	NSCompositeDestinationAtop,
	NSCompositeXOR,
	NSCompositePlusDarker,
	NSCompositeHighlight,
	NSCompositePlusLighter
} NSCompositingOperation;

typedef enum {
	NSWindowBelow = -1,
	NSWindowOut = 0,
	NSWindowAbove = 1
} NSWindowOrderingMode;

typedef enum {
	NSFocusRingOnly,
	NSFocusRingBelow,
	NSFocusRingAbove
} NSFocusRingPlacement;

typedef enum {
	NSFocusRingTypeDefault,
	NSFocusRingTypeNone,
	NSFocusRingTypeExterior
} NSFocusRingType;

typedef int NSWindowDepth;

enum {
	NSAnimationEffectDisappearingItemDefault = 0,
	NSAnimationEffectPoof = 10,
};

typedef enum {
	NSMinXEdge,
	NSMinYEdge,
	NSMaxXEdge,
	NSMaxYEdge
} NSRectEdge;

typedef NSUInteger NSAnimationEffect;

C_API const float NSBlack;
C_API const float NSDarkGray;
C_API const float NSLightGray;
C_API const float NSWhite;

C_API NSString const NSDeviceBlackColorSpace;
C_API NSString const NSDeviceWhiteColorSpace;
C_API NSString const NSDeviceRGBColorSpace;
C_API NSString const NSDeviceCMYKColorSpace;
C_API NSString const NSCalibratedBlackColorSpace;
C_API NSString const NSCalibratedWhiteColorSpace;
C_API NSString const NSCalibratedRGBColorSpace;
C_API NSString const NSNamedColorSpace;
C_API NSString const NSPatternColorSpace;

C_API NSString const NSDeviceIsScreen;
C_API NSString const NSDeviceIsPrinter;
C_API NSString const NSDeviceSize;
C_API NSString const NSDeviceResolution;
C_API NSString const NSDeviceColorSpaceName;
C_API NSString const NSDeviceBitsPerSample;

C_API void NSRectClipList(const NSRect *rects, int count);
C_API void NSRectClip(NSRect rect);

C_API void NSRectFillListWithColors(const NSRect *rects, NSColor **colors, int count);
C_API void NSRectFillListWithGrays(const NSRect *rects, const float *grays, int count);
C_API void NSRectFillList(const NSRect *rects, int count);
C_API void NSRectFill(NSRect rect);

C_API void NSRectFillListUsingOperation(const NSRect *rects, int count, NSCompositingOperation operation);
C_API void NSRectFillUsingOperation(NSRect rect, NSCompositingOperation operation);

C_API void NSFrameRectWithWidth(NSRect rect, CGFloat width);
C_API void NSFrameRectWithWidthUsingOperation(NSRect rect, CGFloat width, NSCompositingOperation operation);
C_API void NSFrameRect(NSRect rect);
C_API void NSDottedFrameRect(NSRect rect);

C_API void NSDrawButton(NSRect rect, NSRect clipRect);
C_API void NSDrawGrayBezel(NSRect rect, NSRect clipRect);
C_API void NSDrawWhiteBezel(NSRect rect, NSRect clipRect);
C_API void NSDrawDarkBezel(NSRect rect, NSRect clipRect);
C_API void NSDrawLightBezel(NSRect rect, NSRect clipRect);
C_API void NSDrawGroove(NSRect rect, NSRect clipRect);

C_API void NSDrawWindowBackground(NSRect rect);

C_API NSRect NSDrawTiledRects(NSRect bounds, NSRect clip, const NSRectEdge *sides, const float *grays, int count);

C_API void NSHighlightRect(NSRect rect);
C_API void NSCopyBits(int gState, NSRect rect, NSPoint point);

C_API void NSBeep(void);

C_API void NSEnableScreenUpdates(void);
C_API void NSDisableScreenUpdates(void);

C_API void NSShowAnimationEffect(NSAnimationEffect effect, NSPoint center, NSSize size,
	id delegate, SEL didEndSelector, void *context);

C_API NSInteger cocoa_status(id instance, const char *selector);
C_API NSInteger cocoa_status_with(id instance, const char *selector, id with, id self);
C_API void cocoa_post(const char *id_class, const char *selector);
C_API id cocoa_get(const char *id_class, const char *selector);
C_API id cocoa_get_with(const char *id_class, const char *selector, id with);
C_API id cocoa_get_status(const char *id_class, const char *selector, NSInteger code);
C_API id cocoa_send(id instance, const char *selector) ;
C_API id cocoa_alloc(const char *id_class) ;
C_API id cocoa_new(const char *id_class) ;
C_API id cocoa_autorelease(const char *id_class);
C_API id cocoa_send_data(id instance, const char *selector, void *data);
C_API id cocoa_send_with(id instance, const char *selector, id data);
C_API id cocoa_send_rect(id instance, const char *selector, float x, float y, float width, float height);
C_API id cocoa_send_status(id instance, const char *selector, NSInteger code);
C_API id cocoa_init(id instance);
C_API id cocoa_alloc_class(Class object);
C_API id cocoa_init_window(int x, int y, int width, int height, int style, int backing, bool defer);
C_API NSView cocoa_content_view(id window);
C_API NSRect cocoa_frame(id window);
C_API NSRect cocoa_bounds(id window);
C_API NSEvent cocoa_next_event(id instance, unsigned long mask, id expiration, id mode, BOOL deqFlag);

C_API NSString cocoa_str(const char *text);
C_API NSInteger cocoa_strlen(NSString str);
C_API BOOL cocoa_str_regex(const char *regexString, NSString stringToEvaluate);
C_API BOOL cocoa_str_has(NSString str, char *match);
C_API NSRange cocoa_str_pos(NSString str, char *match, NSInteger options);
C_API NSString cocoa_sprintf(const char *fmt, ...);
C_API char *cocoa_tochar(NSString str);

C_API NSArray cocoa_array(id instance, ...);
C_API id cocoa_array_index(NSArray arr, NSInteger at);
C_API NSInteger cocoa_array_count(NSArray arr);

C_API NSMutableArray cocoa_array_mutable(void);
C_API void cocoa_append(NSMutableArray arr, id value);

C_API void cocoa_select(id instance, const char *selector) ;
C_API void cocoa_select_handler(id instance, const char *selector, id self, IMP_INT func);

C_API void cocoa_set_rect(id instance, const char *selector, float x, float y, float width, float height);
C_API void cocoa_set_point(id instance, const char *selector, float x, float y) ;
C_API void cocoa_set_size(id instance, const char *selector, float x, float y) ;
C_API void cocoa_set_with(id instance, const char *selector, id with);
C_API void cocoa_set_pair(id instance, const char *selector, id with, id pair);
C_API void cocoa_set(id instance, const char *selector, int value);

C_API NSFont cocoa_font(const char *fontName, float size);
C_API NSDictionary cocoa_dict(NSString key, id value);
C_API NSDictionary cocoa_dictionary(id value, char *key, ...);

C_API void cocoa_menuitem_font(NSMenuItem menuItem, NSDictionary attributes);
C_API NSMenuItem cocoa_menuitem_action(id menu, id append, char *name, void *action, char *key, void *object);
C_API void cocoa_menu_separator(id menu);
C_API void cocoa_menuitem(id menu, id append, const char *name, const char *selector, const char *key);
C_API void cocoa_menuitem_sub(id menu, const char *name, id sub);

C_API void cocoa_impl_func(const char *class_name, const char *register_name, void *function);
C_API NSTextField cocoa_text_field(id gui, ui_field_type kind, char *label, char *field,
	float x, float y, float width, uintptr_t tag);
C_API NSButton cocoa_form_button(id window, char *title, char *action, float x, float y);
C_API void cocoa_check(id window, NSButton, BOOL onOff);

#define dict(obj, key)	((id)obj), ((char *)(key))

#endif
#endif