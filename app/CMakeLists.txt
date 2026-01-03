cmake_minimum_required(VERSION 3.10.0)

project(mapbox-gl-qml-app)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Qml Quick Positioning Sql)

if(NOT Qt${QT_VERSION_MAJOR}_FOUND)
    message(FATAL_ERROR "Qt${QT_VERSION_MAJOR} not found")
endif()

# Find ICU
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED icu-uc icu-io)

# Optional curl SSL
if(USE_CURL_SSL)
    pkg_check_modules(CURL libcurl REQUIRED)
    pkg_check_modules(OPENSSL openssl REQUIRED)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    qml.qrc
)

target_link_libraries(${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Positioning
    Qt${QT_VERSION_MAJOR}::Sql
    QMapLibre
    z
    crypto
    dl
    ${ICU_LIBRARIES}
)

if(USE_CURL_SSL)
    target_link_libraries(${PROJECT_NAME}
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CURL_INCLUDE_DIRS}
        ${OPENSSL_INCLUDE_DIRS}
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${ICU_INCLUDE_DIRS}
)

# QML import path
set(QML_IMPORT_PATH ${CMAKE_SOURCE_DIR}/src/plugin CACHE STRING "QML import path" FORCE)
