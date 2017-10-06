import QtQuick 2.0
import QtPositioning 5.3

//! Panning and pinch implementation on the maps
PinchArea {
    id: pincharea

    //! Holds map object to be controlled
    property var map

    //! Holds previous zoom level value
    property double __oldZoom

    anchors.fill: parent

    //! Calculate zoom level
    function calcZoomDelta(zoom, percent) {
        return zoom + Math.log(percent)/Math.log(2)
    }

    //! Save previous zoom level when pinch gesture started
    onPinchStarted: {
        __oldZoom = map.zoomLevel
    }

    //! Update map's zoom level when pinch is updating
    onPinchUpdated: {
        //map.zoomLevel = calcZoomDelta(__oldZoom, pinch.scale)
        map.setZoomLevel(calcZoomDelta(__oldZoom, pinch.scale), pinch.center)
    }

    //! Update map's zoom level when pinch is finished
    onPinchFinished: {
        //map.zoomLevel = calcZoomDelta(__oldZoom, pinch.scale)
        map.setZoomLevel(calcZoomDelta(__oldZoom, pinch.scale), pinch.center)
    }

    //! Map's mouse area for implementation of panning in the map
    MouseArea {
        id: mousearea

        //! Property used to indicate if panning the map
        property bool __isPanning: false

        //! Last pressed X and Y position
        property int __lastX: -1
        property int __lastY: -1

        //! Panned distance to distinguish panning from clicks
        property int __pannedDistance: 0

        anchors.fill : parent

        function isPanning() {
            return __isPanning && __pannedDistance > 0;
        }

        //! When pressed, indicate that panning has been started and update saved X and Y values
        onPressed: {
            __isPanning = true
            __lastX = mouse.x
            __lastY = mouse.y
            __pannedDistance = 0
        }

        //! When released, indicate that panning has finished
        onReleased: {
            __isPanning = false
        }

        //! Move the map when panning
        onPositionChanged: {
            if (__isPanning) {
                var dx = mouse.x - __lastX
                var dy = mouse.y - __lastY
                map.pan(dx, dy)
                __lastX = mouse.x
                __lastY = mouse.y
                __pannedDistance += Math.abs(dx) + Math.abs(dy);
            }
        }

        //! When canceled, indicate that panning has finished
        onCanceled: {
            __isPanning = false;
        }

        onWheel: {
            map.setZoomLevel( map.zoomLevel + 0.2 * wheel.angleDelta.y / 120, Qt.point(wheel.x, wheel.y) )
        }

        onPressAndHold: !isPanning() && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), "mouse onPressAndHold")

        onClicked: !isPanning() && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), "mouse onClicked")

        onDoubleClicked: {
            if (!isPanning())
                map.center = gps.position.coordinate;
        }
    }

    Connections {
        target: map

        onReplyCoordinateForPixel: {
            if (tag === "mouse onPressAndHold") {
                console.log("Mouse pressed & hold at " + geocoordinate)
                return;
            }

            if (tag === "mouse onClicked") {
                console.log("Mouse clicked at " + geocoordinate + " precision: " +  degLatPerPixel + " " + degLonPerPixel)
                return;
            }
        }
    }
}
