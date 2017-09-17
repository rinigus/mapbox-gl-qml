import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtPositioning 5.0

import QQuickItemMapboxGL 1.0

ApplicationWindow {
    title: "Mapbox GL QML example"
    width: 1024
    height: 768
    visible: true

    MapboxMap {
        id: map
        anchors.fill: parent

        center: QtPositioning.coordinate(60.170448, 24.942046) // Helsinki
        zoomLevel: 12.25
        minimumZoomLevel: 0
        maximumZoomLevel: 20
        pixelRatio: 1.5

        bearing: bearingSlider.value
        pitch: pitchSlider.value

        //accessToken: "INSERT_THE_TOKEN_OR_DEFINE_IN_ENVIRONMENT"
        cacheDatabaseMaximalSize: 20*1024*1024
        cacheDatabasePath: "/tmp/mbgl-cache.db"

        //styleUrl: "mapbox://styles/mapbox/satellite-streets-v10"

        MouseArea {
            id: mouseArea
            anchors.fill: parent

            property var lastX: 0
            property var lastY: 0

            onWheel: {
                //map.zoomLevel += 0.2 * wheel.angleDelta.y / 120
                map.setZoomLevel( map.zoomLevel + 0.2 * wheel.angleDelta.y / 120, Qt.point(wheel.x, wheel.y) )
            }

            onPressed: {
                lastX = mouse.x
                lastY = mouse.y
            }

            onPositionChanged: {
                map.pan(mouse.x - lastX, mouse.y - lastY)

                lastX = mouse.x
                lastY = mouse.y
            }
        }
    }

    Rectangle {
        anchors.fill: menu
        anchors.margins: -20
        radius: 30
        clip: true
    }

    Column {
        id: menu

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 30

        Label {
            text: "Bearing:"
        }

        Slider {
            id: bearingSlider

            anchors.left: parent.left
            anchors.right: parent.right
            maximumValue: 180
        }

        Label {
            text: "Pitch:"
        }

        Slider {
            id: pitchSlider

            anchors.left: parent.left
            anchors.right: parent.right
            maximumValue: 60
        }
    }

}
