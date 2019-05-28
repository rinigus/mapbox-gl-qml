TEMPLATE = lib
CONFIG += qt plugin
QT += qml quick positioning location network gui sql

CONFIG += c++14

use_curl_ssl {
   CONFIG += link_pkgconfig
   DEFINES += USE_CURL_SSL
   PKGCONFIG += libcurl openssl
}

TARGET = qmlmapboxglplugin

include(mapbox-gl-qml.pri)

SOURCES += \
    src/plugin/mapboxglextensionplugin.cpp
HEADERS += \
    src/plugin/mapboxglextensionplugin.h

INCLUDEPATH += src/plugin
    
LIBS += -lqmapboxgl -lz -L/opt/gcc6/lib -static-libstdc++

target.path=$$[QT_INSTALL_QML]/MapboxMap
INSTALLS += target

qmldirtarget.path=$$[QT_INSTALL_QML]/MapboxMap
qmldirtarget.files=src/plugin/qmldir src/plugin/MapboxMapGestureArea.qml
INSTALLS += qmldirtarget
