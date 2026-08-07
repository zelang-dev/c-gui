#[=======================================================================[

FindGui
------------

Find the Gui - dual webclient/server processor.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following imported targets:

GUI::MINI
    The Gui library, if found.

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``GUI_FOUND``
    System has the Gui library.
``GUI_INCLUDE_DIR``
    The Gui include directory.
``GUI_LIBRARY``
    The Gui library.
``GUI_VERSION``
    This is set to $major.$minor.$revision (e.g. 2.6.8).

Hints
^^^^^

Set GUI_ROOT_DIR to the root directory of an Gui installation.

]=======================================================================]

# Find TLS Library
find_library(gui_LIBRARY
    NAMES
        gui
        libgui
)
mark_as_advanced(gui_LIBRARY)

# Find Include Path
find_path(gui_INCLUDE_DIR
    NAMES gui.h
)
mark_as_advanced(gui_INCLUDE_DIR)

include (FindPackageHandleStandardArgs)
# Set Find Package Arguments
find_package_handle_standard_args(gui
    FOUND_VAR gui_FOUND
    REQUIRED_VARS GUI_LIBRARY GUI_INCLUDE_DIR
    VERSION_VAR GUI_VERSION
    HANDLE_COMPONENTS
        FAIL_MESSAGE
        "Could NOT find Gui, try setting the path to Gui using the GUI_ROOT_DIR environment variable"
)

set(GUI_FOUND ${gui_FOUND})
set(GUI_LIBRARY ${GUI_LIBRARY})

# Gui Found
if(GUI_FOUND)
	set(GUI_INCLUDE_DIRS ${GUI_INCLUDE_DIR})
	set(GUI_LIBRARIES ${GUI_LIBRARY})
    if(NOT TARGET GUI::MINI)
        add_library(GUI::MINI UNKNOWN IMPORTED)
        set_target_properties(GUI::MINI PROPERTIES
			IMPORTED_LOCATION "${GUI_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${GUI_INCLUDE_DIRS}"
        )
    endif()
endif(GUI_FOUND)
