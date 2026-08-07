cmake_minimum_required (VERSION 3.22...5.1.0)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
	cmake_policy(SET CMP0091 NEW)
endif()

project(gui
	VERSION 0.0.1
	DESCRIPTION "Minimal Cross-Platform `C` GUI"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 90)
set(PROJECT c-gui)

include(FetchContent)
include(CheckLibraryExists)
include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(GNUInstallDirs)
include(CTest)

set(BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/built")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_SHARED_LIBS "Build the library as a shared (dynamically-linked) " OFF)

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
set(CMAKE_CONFIGURATION_TYPES=Debug;Release)

if (MSVC)
    if("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "X86")
      set(PROJECT__ARCH "i686")
    elseif("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "x64")
      set(PROJECT__ARCH "x86_64")
    elseif("${MSVC_C_ARCHITECTURE_ID}" STREQUAL "ARM")
      set(PROJECT__ARCH "arm")
    else()
      message(FATAL_ERROR "Failed to determine the MSVC target architecture: ${MSVC_C_ARCHITECTURE_ID}")
    endif()
else()
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -dumpmachine
      RESULT_VARIABLE RESULT
      OUTPUT_VARIABLE PROJECT__ARCH
      ERROR_QUIET
    )
    if (RESULT)
      message(FATAL_ERROR "Failed to determine target architecture triplet: ${RESULT}")
    endif()
    string(REGEX MATCH "([^-]+).*" ARCH_MATCH ${PROJECT__ARCH})
    if (NOT CMAKE_MATCH_1 OR NOT ARCH_MATCH)
      message(FATAL_ERROR "Failed to match the target architecture triplet: ${PROJECT__ARCH}")
    endif()
    set(PROJECT__ARCH ${CMAKE_MATCH_1})
endif()
message(STATUS "Target architecture - ${PROJECT__ARCH}")

if(UNIX)
    if(APPLE)
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D USE_DEBUG")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wabi -D _DARWIN_UNLIMITED_SELECT=1 -D _DARWIN_USE_64_BIT_INODE=1")
    else()
		set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -rdynamic -no-pie -D USE_DEBUG")
    endif()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format -fno-strict-aliasing -Wstrict-prototypes")
else()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D USE_DEBUG")
endif()

if(APPLE)
	if(BUILD_SHARED_LIBS)
		add_library(${PROJECT_NAME} SHARED
			${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c)
	else()
		add_library(${PROJECT_NAME} STATIC
			${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c)
	endif()
	find_library(COCOA_LIBRARY Cocoa required)
	target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY})
else()
	if(WIN32)
		if(BUILD_SHARED_LIBS)
			add_library(${PROJECT_NAME} SHARED
				${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c
				${CMAKE_CURRENT_SOURCE_DIR}/src/re.c
			)
		else()
			add_library(${PROJECT_NAME} STATIC
				${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c
				${CMAKE_CURRENT_SOURCE_DIR}/src/re.c
			)
		endif()
		target_link_libraries(${PROJECT_NAME} gdi32 comctl32 )
	else()
		find_package(X11 REQUIRED COMPONENTS Xmu Xt)
		find_package(OpenGL REQUIRED COMPONENTS GLX)
		file(GLOB gui_files
			${CMAKE_CURRENT_SOURCE_DIR}/src/Linux/*.c
			${CMAKE_CURRENT_SOURCE_DIR}/src/Linux/Xaw95/*.c)
		if(BUILD_SHARED_LIBS)
			add_library(${PROJECT_NAME} SHARED
				${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c
				${CMAKE_CURRENT_SOURCE_DIR}/src/re.c
				${gui_files})
		else()
			add_library(${PROJECT_NAME} STATIC
				${CMAKE_CURRENT_SOURCE_DIR}/src/gui.c
				${CMAKE_CURRENT_SOURCE_DIR}/src/re.c
				${gui_files})
		endif()
		target_compile_definitions(${PROJECT_NAME} -DX11_WINDOWS_ICON="${CMAKE_SOURCE_DIR}/resources/icon_32x32.xpm")
		target_link_libraries(${PROJECT_NAME} X11::Xmu X11::Xt X11::X11 OpenGL::GL OpenGL::GLU)
	endif()
endif()
add_library(GUI::MINI ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set_target_properties(${PROJECT_NAME} PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	ARCHIVE_OUTPUT_NAME ${PROJECT_NAME}
	EXPORT_NAME ${PROJECT_NAME}
	VERSION  ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(NOT CMAKE_VERSION VERSION_LESS 3.27.0)
	set_target_properties(${PROJECT_NAME} PROPERTIES DLL_NAME_WITH_SOVERSION TRUE)
endif()

set(_fmt TGZ)
if(WIN32)
  set(_fmt ZIP)
endif()

set(CPACK_GENERATOR ${_fmt})
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_NUGET_COMPONENT_INSTALL ON)
set(CPACK_WIX_COMPONENT_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE 1)
set(CPACK_VERBATIM_VARIABLES YES)

set(CPACK_PACKAGE_VENDOR "https://github.com/zelang-dev/${PROJECT}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

set(CMAKE_INSTALL_CONFIG_NAME ${CMAKE_BUILD_TYPE})
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    CACHE PATH "Location of header files" )

set(SYSCONFIG_INSTALL_DIR ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    CACHE PATH "Location of configuration files" )

configure_package_config_file(${PROJECT_NAME}Config.cmake.in
  	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/"${PROJECT_NAME}"
	PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion )

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/Package")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_NAME}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})

set(CPACK_PACKAGE_CONTACT "lstubbs@zelang.dev")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Lawrence Stubbs <${CPACK_PACKAGE_CONTACT}>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${PROJECT__ARCH}")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "OpenGL, OpenGLx")

set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "${PROJECT_NAME}")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION ${PROJECT_DESCRIPTION})

set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

set(CPACK_PACKAGE_VENDOR "https://github.com/zelang-dev/${PROJECT}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_DESCRIPTION})

set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_URL "https://zelang-dev.github.io/${PROJECT}/")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "${PROJECT__ARCH}")
set(CPACK_RPM_PACKAGE_REQUIRES "OpenGL, OpenGLx")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" )

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/resources")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_SOURCE_DIR}/resources)
endif()

add_subdirectory(resources)