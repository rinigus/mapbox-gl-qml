TEMPLATE = lib
CONFIG += qt plugin
QT += qml quick positioning

CONFIG += c++14
QMAKE_CXX=/opt/gcc6/bin/g++
QMAKE_CC=/opt/gcc6/bin/gcc
QMAKE_LINK=/opt/gcc6/bin/g++

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
qmldirtarget.files=src/plugin/qmldir
INSTALLS += qmldirtarget
