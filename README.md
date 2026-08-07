# c-gui

A *minimal* cross-platform **C** `GUI` library.

The major dependency needed is what the O.S provides natively.
For *aesthetic* reasons **Linux** will require some additional libraries `OpenGL` besides `X11`.

## Linux

This started out first on **Linux** with no prior understanding of X11 programming inferface, mainly inspired by following the pattern layout in [Minimal cross-platform graphics](https://zserge.com/posts/fenster). Digging more into *X11 universe*, the aesthetics part seemed to be a big after thought. Plenty of info on **X11 API**, but no official code tutorials. The closet that has actually example code is [X-windows: programming with Athena widgets](https://ergodic.ugr.es/cphys_pedro/unix/intro.html).

* Began with a simplier menu layout derived from [X11 Menus (how to)](https://www.linuxquestions.org/questions/programming-9/x11-menus-how-to-839904/), the source for `OpenGL` dependency.
* Switched to an alterative [Athena Widgets/Xaw implementations](https://forums.freebsd.org/threads/athena-widgets-xaw-implementations.81588/) toolkit, parts of this *library* includes various aspects of [Survey of Widget Sets](http://www.efalk.org/Widgets/).

## Windows

After getting a basic functional **Linux** startup running under `WSL2`. The same **skeleton app** needed major refactoring for **Windows**, API changed to be simplier and more.

* I followed the [theForger's Win32 API Programming Tutorial](https://winprog.org/tutorial/).
* And [Windows API tutorial](https://zetcode.com/gui/winapi/).

## Apple macOS

**Apple macOS** required the most reading, and a lot of trial and error, going from *objective-c* to *c* and *back*. There is one question to ask [Can you build a foundation with a spoon?](http://stackoverflow.com/questions/10289890/how-to-write-ios-app-purely-in-c#comment13239523_10289913).

* [Minimalist Cocoa programming](https://www.cocoawithlove.com/2010/09/minimalist-cocoa-programming.html)
* [Cocoa in Pure C](https://github.com/ColleagueRiley/Cocoa-in-Pure-C)
* [The Objective-C Programming Language](https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/ObjectiveC/Chapters/ocObjectsClasses.html)
* [Cocoa Programming/Objective-C basics](https://en.wikibooks.org/wiki/Cocoa_Programming/Objective-C_basics)
* [Understanding the Objective-C Runtime](https://cocoasamurai.blogspot.com/2010/01/understanding-objective-c-runtime.html)
* [Design of a multi-platform app](https://www.cocoawithlove.com/2010/04/design-of-multi-platform-app-using.html)![applogic](applogic.png)

The same **skeleton app** overhauled to include most **Apple macOS API** automatic logic handling, which are shortcuts to various aspects of [Cocoa examples without StoryBoard](https://github.com/gammasoft71/Examples_Cocoa) and [Cocoa macOS Examples [Objective-C]](https://github.com/NikolaGrujic91/Cocoa-macOS-Examples). Cocoa [AppKit](https://developer.apple.com/documentation/appkit/) controls without StoryBoard only by programming code (objective-c).

> NOTE: The current **Linux** and **Windows** API needs refactoring to match **Apple** behavior, now broken. The **skeleton app** *should have no platform specific code*. **Linux** will be the most challenging part without resorting to **GTK**, **Qt**. I have *no direct* plans to add, **PR** are welcome.
> This should be must *lighter* and *easier* to follow than something like [Cross-Platform C SDK - NAppGUI](https://github.com/frang75/nappgui_src).

## Usage/installation

[CMake](https://cmake.org) `FetchContent` and `find_package` is use here to setup your project `App`. Your **WILL** need to *modify* every *file* in [resources](./resources/) folder. The folder will be created in your *root directory* if it doesn't exist. This project started out inside **repo** [Httpi](https://github.com/zelang-dev/httpi), all prior commits are there. This is provided as *quick way* to *reproduce* an *GUI* `App` with little effort.

```sh
find_package(gui QUIET CONFIG)
if(NOT gui_FOUND)
    FetchContent_Declare(gui
        URL https://github.com/zelang-dev/c-gui/archive/refs/heads/main.zip
        #URL https://github.com/zelang-dev/c-gui/archive/refs/tags/v0.0.1.zip
        #URL_MD5 6e9756dee0ef5903d850dc021d5df725
    )
    FetchContent_MakeAvailable(gui)
endif()
target_include_directories(your_project
 PRIVATE $<BUILD_INTERFACE:${GUI_INCLUDE_DIR} $<INSTALL_INTERFACE:${GUI_INCLUDE_DIR})
target_link_libraries(your_project PUBLIC GUI::MINI)
```
