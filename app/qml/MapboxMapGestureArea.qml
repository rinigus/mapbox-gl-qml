import QtQuick 2.0
import QtPositioning 5.3

Item {

    //////////////////////////////////////
    /// Exported properties and signals

    /// Must be specified on construction: Holds map object to be controlled
    property var map

    /// Whether clickedGeo signal is active
    property bool activeClickedGeo: false

    /// Whether doubleClickedGeo signal is active
    property bool activeDoubleClickedGeo: false

    /// Whether pressAndHoldGeo signal is active
    property bool activePressAndHoldGeo: false

    /// emitted on clicked event
    signal clicked(var mouse);

    /// emitted on clicked event with geographical coordinates
    signal clickedGeo(var geocoordinate, real degLatPerPixel, real degLonPerPixel);

    /// emitted on doubleClicked event
    signal doubleClicked(var mouse);

    /// emitted on doubleClicked event with geographical coordinates
    signal doubleClickedGeo(var geocoordinate, real degLatPerPixel, real degLonPerPixel);

    /// emitted on pressAndHold event
    signal pressAndHold(var mouse);

    /// emitted on pressAndHold event with geographical coordinates
    signal pressAndHoldGeo(var geocoordinate, real degLatPerPixel, real degLonPerPixel);

    ////////////////////////////////////////////////
    /// Implementation details

    id: mpbxGestureArea

    anchors.fill: parent

    PinchArea {
        id: pincharea

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
            map.setZoomLevel(calcZoomDelta(__oldZoom, pinch.scale), pinch.center)
        }

        //! Update map's zoom level when pinch is finished
        onPinchFinished: {
            map.setZoomLevel(calcZoomDelta(__oldZoom, pinch.scale), pinch.center)
        }

        //! Map's mouse area for implementation of panning in the map
        MouseArea {
            id: mousearea

            property var constants: QtObject {
                property string eventPrefix: "MAPBOX MAP GESTURE AREA - "
            }

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

            /////////////////////////////////////////////////////////
            /// exported signals

            onClicked: {
                if (isPanning()) return;
                activeClickedGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onClicked");
                mpbxGestureArea.clicked(mouse);
            }

            onDoubleClicked: {
                if (isPanning()) return;
                activeClickedGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onDoubleClicked");
                mpbxGestureArea.doubleClicked(mouse);
            }

            onPressAndHold: {
                if (isPanning()) return;
                activePressAndHoldGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onPressAndHold");
                mpbxGestureArea.pressAndHold(mouse);
            }
        }

        Connections {
            target: map

            onReplyCoordinateForPixel: {
                if (tag === mousearea.constants.eventPrefix + "onClicked")
                    mpbxGestureArea.clickedGeo(geocoordinate, degLatPerPixel, degLonPerPixel);
                else if (tag === mousearea.constants.eventPrefix + "onDoubleClicked")
                    mpbxGestureArea.doubleClickedGeo(geocoordinate, degLatPerPixel, degLonPerPixel);
                else if (tag === mousearea.constants.eventPrefix + "onPressAndHold")
                    mpbxGestureArea.pressAndHoldGeo(geocoordinate, degLatPerPixel, degLonPerPixel);
            }
        }
    }
}
