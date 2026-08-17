if(DEFINED webview2)
    find_path(webview2_INCLUDE_DIR WebView2.h
        PATHS
            "${webview2}/build/native"
            "${webview2}"
        PATH_SUFFIXES include
        NO_CMAKE_FIND_ROOT_PATH)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(webview2 REQUIRED_VARS webview2_INCLUDE_DIR)

if(webview2_FOUND)
    if(NOT TARGET WEBVIEW2::HEADERS)
        add_library(WEBVIEW2::HEADERS INTERFACE IMPORTED)
        set_target_properties(WEBVIEW2::HEADERS PROPERTIES
			IMPORTED_LOCATION "${WEBVIEW2_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${WEBVIEW2_INCLUDE_DIR}")
    endif()
endif()
