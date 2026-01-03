cmake_minimum_required(VERSION 3.10.0)

project(mapbox-gl-qml
	VERSION 3.1.0
	DESCRIPTION "Unofficial Mapbox GL Native bindings for Qt QML")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_CURL_SSL "Use curl SSL" OFF)
option(USE_FBO "Use map rendering using FBO" ON)
option(BUILD_DEMO_APP "Build the demo application" OFF)

# Render backend option
set(RENDER_BACKEND "opengl" CACHE STRING "Render backend to use (opengl, metal, vulkan)")
set_property(CACHE RENDER_BACKEND PROPERTY STRINGS opengl metal vulkan)

# Validate RENDER_BACKEND value
if(NOT RENDER_BACKEND STREQUAL "opengl" AND
   NOT RENDER_BACKEND STREQUAL "metal" AND
   NOT RENDER_BACKEND STREQUAL "vulkan")
    message(FATAL_ERROR "Invalid RENDER_BACKEND value: ${RENDER_BACKEND}. Must be one of: opengl, metal, vulkan")
endif()

# Set render backend compiler definitions
if(RENDER_BACKEND STREQUAL "opengl")
 add_definitions(-DMLN_RENDER_BACKEND_OPENGL=1)
elseif(RENDER_BACKEND STREQUAL "metal")
 add_definitions(-DMLN_RENDER_BACKEND_METAL=1)
elseif(RENDER_BACKEND STREQUAL "vulkan")
 add_definitions(-DMLN_RENDER_BACKEND_VULKAN=1)
endif()

include(FindPkgConfig)
include(FeatureSummary)
include(GNUInstallDirs)

# Find Qt
if("${QT_VERSION_MAJOR}" STREQUAL "")
    find_package(Qt6 QUIET COMPONENTS Core)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Core)
        set(QT_VERSION_MAJOR 5)
    endif()
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
endif()
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Quick Positioning Sql Svg REQUIRED)

set(QT_VERSION ${Qt${QT_VERSION_MAJOR}_VERSION})

if("${QT_VERSION}" STREQUAL "")
    message(FATAL_ERROR "Qt version unknown")
endif()

message(STATUS "Using Qt${QT_VERSION_MAJOR} (${QT_VERSION})")

find_package(QMapLibre REQUIRED)

if(USE_CURL_SSL)
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(CURL libcurl IMPORTED_TARGET)
	pkg_check_modules(OPENSSL openssl IMPORTED_TARGET)

	set_property(GLOBAL APPEND PROPERTY _CMAKE_libcurl_TYPE REQUIRED)
	set_property(GLOBAL APPEND PROPERTY _CMAKE_openssl_TYPE REQUIRED)

	if(CURL_FOUND)
		set_property(GLOBAL APPEND PROPERTY PACKAGES_FOUND libcurl)
	else()
		set_property(GLOBAL APPEND PROPERTY PACKAGES_NOT_FOUND libcurl)
	endif()

	if(OPENSSL_FOUND)
		set_property(GLOBAL APPEND PROPERTY PACKAGES_FOUND openssl)
	else()
		set_property(GLOBAL APPEND PROPERTY PACKAGES_NOT_FOUND openssl)
	endif()
endif()

add_subdirectory(src)

if(BUILD_DEMO_APP)
    add_subdirectory(app)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
