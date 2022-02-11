cmake_minimum_required(VERSION 3.6.0)

project(mapbox-gl-qml
	VERSION 1.7.6
	DESCRIPTION "Unofficial Mapbox GL Native bindings for Qt QML")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_CURL_SSL "Use curl SSL" OFF)
option(USE_FBO "Use map rendering using FBO" ON)

include(FindPkgConfig)
include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MIN_VERSION "5.6.0")
find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Gui Quick Positioning Sql Svg REQUIRED)
find_package(QMapboxGL REQUIRED)

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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

