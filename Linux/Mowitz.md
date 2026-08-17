# This is the Mowitz ("More widgets) library

*From:* <https://github.com/UlricE/Mowitz>

The project's goal is to create a library of widgets for X applications
to use. The widgets have been snarfed from various sources and are
all open source (GPL or MIT licenses).

## Available widgets

- Animator
- Canvas
- Check
- Combo
- Frame
- Handle
- Html
- Image
- ListTree
- Notebook
- Richtext
- Rudegrid
- Ruler
- Slider
- Spinner
- Tabbing
- Table
- Tabs
- TextField
- Tooltip
- VSlider

And a complete menu kit.

The original reason for creating this library is that the Athena
widget set lacks a lot of widgets that would be useful, and many
of the existing widgets leave a lot to be desired in appearance
and behaviors.

Once upon a time there was a project called the Free Widget Foundation.
It started out with goals similar to those of Mowitz, but later
turned into a project to create a complete, standalone widget set.
That is not something that will happen here.

## Namespace

All external symbols are prefixed with "Mw" in order to avoid
name space clashes with other libraries. So for example, the
spinner widget is called MwSpinner.

## Applications using Mowitz

Since most of these widgets were previously distributed with
Siag Office, it is natural that the upcoming release of that
package will use (and require) the Mowitz library.

Many existing X applications can be trivially updated to use
the widgets in the Mowitz library.

## Utility routines

In addition to the widgets themselves, Mowitz includes several
utility functions that are used within the library, but useful
to application writers as well. The functionality includes:

- Loading and caching pixmaps
- Allocating and caching pixel values

## Mowitz à la carte

It is possible to pick and choose among the widgets within Mowitz.
Normally there is no reason to do so, but it can be a way to add
a single widget to an application without having to depend on
another library. It can also be a way to avoid licensing issues.

To do this, copy the MwUtils.c and MwUtils.h files to the application,
plus the files for the widget. All widgets use the routines in
MwUtils, so it must always be included.

Some widgets are subclassed from other widgets in the library. The
Handle widget, for example, is subclassed from Frame. In that case,
both widgets must be included.

## Widget hierarchy

Names in brackets are Intrinsic widgets, not part of Mowitz.

[RectObj]
  |-BaseME
  |  |-LabelME
  |  |  |-CheckME
  |  |  |-MBButton
  |  |  `-SubME
  |`-LineME
[Core]
  |-Base
  |  `-SButton
  |`-Button
  |        `-MenuButton
  |-Canvas
  |-Check
  |-[Composite]
  |   |-Animator
  |   |-BaseComp
  |   |-[Constraint]
  |   |   |-BaseConst
  |   |   |   |-Row
  |   |   |`-MenuBar
  |   |   |-Rudegrid
  |   |   |  `-Filesel
  |   |`-Tabs
  |   |-Frame
  |   |  |-Combo
  |   |  |-Handle
  |   |  `-Spinner
  |   |-Richtext
  |   |-[Shell]
  |   |`-[OverrideShell]
  |   |       |-Menu
  |   |       |-PopText
  |   |       `-Tooltip
  |`-Table
  |-Image
  |-ListTree
  |-Notebook
  |-Ruler
  |-Slider
  |  `-VSlider
  |-Tabstop
  `-TextField

## Authors

Ondrejicka Stefan: Menus
Edward A. Falk: Frame, Ruler, Slider, VSlider, Tabs
Robert W. McMullen: ListTree, TextField
Ulric Eriksson: Animator, Canvas, Check, Combo, Filesel,
  Handle, Image, Notebook, Richtext,
  Rudegrid, Spinner, Tabbing, Table, Tabs, Tabstop, Tooltip

## ChangeLog

070830 Two patches from Pascal Terjan fixes buffer overflows
 in MwXFilesel.c and MwXFormat.c.
 Released 0.3.1.

040108 Added John Cwikla's XCC code. Prepended Mw to all files
 and external symbols. This adds "Color Contexts" which
 allow easy colour management with any visual. Anything
 within Mowitz that calls MwAllocColor or MwQueryColor
 will be rewritten to use color contexts instead.

030919 Cleaned up the code for release.
 Released 0.3.0.

030304 New widget in MwApplicationShell.c is subclassed from
 applicationShellWidgetClass, but has callbacks for
 OffiX and Xdnd drops.

030209 MwMenu.c: catch X errors in XGrabButton. Needed for OpenWindows,
 discovered by James B. Hiller <jhiller@visi.net>.
 Released 0.2.3.

030128 Changed malloc to MwMalloc, realloc to MwRealloc and
 free to MwFree in MwRudegrid.c.

030117 A freak hack in MwRichtext.c keeps Siag Office from crashing
 on Solaris.

020506 Patch from Petr Mladek <pmladek@suse.cz> handles the case when
 XOpenIM returns NULL because no input method can be opened.
 Release 0.2.1.

020315 Added check to acinclude.m4 to see that the library chosen
 for XAWLIB is actually available, as suggested by
 Edward Avis <ed@membled.com>.
 Another check to define NARROWPROTO only for systems that need it.

020223 Use environment variable BROWSER to get default help browser.
 Netscape is used if BROWSER is not set.

020215 Changed mkt1cfg so it generates the right path for font config.
 Fixed up the t1lib code.

020209 Moved the Html widget to the [Kylie](https://siag.nu/kylie/) project, so it can mature
 in an application rather than a library that Siag Office
 depends on.

020131 Added the HTML parser from [myhtml](https://siag.nu/myhtml/).

020121 Fonts and configuration from Siag Office. Environment variable
 MOWITZ_DATA tells us where it is [/usr/local/share/Mowitz].
020120 Added Animator, Richtext and Table from Siag Office.

020119 Added and cleaned up format code from Siag Office.
 Changed beNiceToColormap everywhere.

020105 Seems like there were remaining cfree's. Removed them.
 Also added --with-xawm configure option, like in Siag Office.
 Released 0.1.1.

020105 OK, I've had it with the file selector. Reverted to 0.0.1 version.
 That one seems broken as well. Reverted to the (non-widget) code
 from Siag Office 3.4.10.
 Added Check menu entry.
 Released 0.1.0.

020103 Changed the default MwMalloc error message.
 Increased MwMalloc paranoia level to 3 in allwidgets.c

011230 Clear out private Filesel resources in Initialize.
 Header fixer-upper in MwRudegridP.h to compile on Solaris.

011230 Added MwCheckME widget.
 Released 0.0.1.

011226 New function MwSetIcon to set application icon.
 New function MwHighlightInit creates two actions to show
 highlighting using the 3d shadow width.
 Added OffiX DND code.
 New Tabstop widget (previously Ruler in Siag Office).

011225 Added more stuff from Siag Office.

011222 Fixed geometry initialisation code for Spinner and Combo.

011221 Updated allwidgets example.

011220 Updated automake, autoconf and libtool.

011218 Added ListTree. New examples: listtree and listtree2.
 Added Slider, VSlider and Ruler.
 New header Mowitz.h includes all other public headers.

011217 Spinner: Removed Rudegrid (subclass directly from Frame).
 Added Check, Tabs and Notebook widgets from Siag Office.

011216 Canvas: Added Initialize method to set initial size.
 Default size is now 50x50.
 Replaced expose resource with XtNcallback callback list.
 Combo: Replaced the text_cb and list_cb resources with the
 callback lists XtNtextCallback and XtNlistCallback.
 Frame: Changed default shadow style to Lowered.
 Handle: Replaced detach and attach resources with callback
 lists XtNdetachCallback and XtNattachCallback.
 Image: Added Initialize method to set initial size (50x50).
 Rudegrid: Set default size (100x100).

011215 Added Nws menu widgets.

011214 Subclassed Combo from Frame rather than Composite.
 Set minimum size in Initialize.

011213 Started project by breaking out lots of code from Siag Office.
