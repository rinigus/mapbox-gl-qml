### Sets QT_INSTALL_QML to the directory where QML Plugins should be installed
function(FindQtInstallQml)
    find_program(QMAKE NAMES qmake-qt5 qmake)
    if(NOT QMAKE)
	message(FATAL_ERROR "qmake not found")
    endif()
    execute_process(
        COMMAND ${QMAKE} -query QT_INSTALL_QML
        OUTPUT_VARIABLE PROC_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(QT_INSTALL_QML ${PROC_RESULT} PARENT_SCOPE)
endfunction()

set(SRC
	qquickitemmapboxgl.cpp
	qsgmapboxglnode.cpp
	qsgtextureplain.cpp
	qmapboxsync.cpp
	plugin/mapboxglextensionplugin.cpp)
set(HEADERS
	qmapboxsync_p.h
	include/qsgmapboxglnode.h
	include/qquickitemmapboxgl.h
	include/qsgtextureplain.h
	plugin/mapboxglextensionplugin.h)

add_library(qmlmapboxglplugin SHARED ${SRC} ${HEADERS})

target_include_directories(qmlmapboxglplugin PRIVATE include src)

target_link_libraries(qmlmapboxglplugin PRIVATE
	Qt5::Gui
	Qt5::Qml
	Qt5::Quick
	Qt5::Positioning
	Qt5::Sql
        Qt5::Svg
	QMapboxGL)

if(USE_CURL_SSL)
	add_definitions(-DUSE_CURL_SSL=1)
	target_link_libraries(qmlmapboxglplugin PRIVATE
		PkgConfig::CURL
		PkgConfig::OPENSSL)
endif()

if(USE_FBO)
        add_definitions(-DUSE_FBO=1)
endif()

if(NOT DEFINED QT_INSTALL_QML)
  FindQtInstallQml()
  message(STATUS "Found QT_INSTALL_QML path: ${QT_INSTALL_QML}")
endif()

install(FILES 
	plugin/MapboxMapGestureArea.qml
	plugin/qmldir
	DESTINATION ${QT_INSTALL_QML}/MapboxMap/)
install(TARGETS qmlmapboxglplugin DESTINATION ${QT_INSTALL_QML}/MapboxMap/)
