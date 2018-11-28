import QtQuick 2.0
import QtQuick.Window 2.2
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

    /// Whether to snap zoom gestures to integer zoom levels
    property bool integerZoomLevels: false

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

    /// emitted on released event
    signal released(var mouse);

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
            var newZoom = calcZoomDelta(__oldZoom, pinch.scale)
            if (integerZoomLevels) {
                if (newZoom < __oldZoom) {
                    newZoom = newZoom % 1 < 0.75 ?
                        Math.floor(newZoom) : Math.ceil(newZoom)
                } else if (newZoom > __oldZoom) {
                    newZoom = newZoom % 1 > 0.25 ?
                        Math.ceil(newZoom) : Math.floor(newZoom)
                }
            }
            map.setZoomLevel(newZoom, pinch.center)
        }

        //! Map's mouse area for implementation of panning in the map
        MouseArea {
            id: mousearea

            property var constants: QtObject {
                property string eventPrefix: "MAPBOX MAP GESTURE AREA - "
            }

            //! Property used to indicate if panning the map
            property bool __isPanning: false

            //! Property used to indicate if touching the map
            property bool __isTouching: false

            //! Last pressed X and Y position
            property int __lastX: -1
            property int __lastY: -1

            //! Required distance to be detected as panning
            property int __panningThreshold: Screen.pixelDensity * 3

            anchors.fill : parent

            //! When pressed, indicate that touching has been started and update saved X and Y values
            onPressed: {
                __isPanning = false
                __isTouching = true
                __lastX = mouse.x
                __lastY = mouse.y
            }

            //! When released, indicate that touching has finished
            onReleased: {
                __isTouching = false;
                mpbxGestureArea.released(mouse);
            }

            //! Move the map when panning
            onPositionChanged: {
                if (__isTouching) {
                    var dx = mouse.x - __lastX
                    var dy = mouse.y - __lastY
                    var dist = Math.sqrt(dx * dx + dy * dy)
                    if (!__isPanning && dist > __panningThreshold)
                        __isPanning = true
                    if (__isPanning) {
                        map.pan(dx, dy)
                        __lastX = mouse.x
                        __lastY = mouse.y
                    }
                }
            }

            //! When canceled, indicate that panning has finished
            onCanceled: {
                __isTouching = false;
            }

            onWheel: {
                map.setZoomLevel( map.zoomLevel + 0.2 * wheel.angleDelta.y / 120, Qt.point(wheel.x, wheel.y) )
            }

            /////////////////////////////////////////////////////////
            /// exported signals

            onClicked: {
                if (__isPanning) return;
                activeClickedGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onClicked");
                mpbxGestureArea.clicked(mouse);
            }

            onDoubleClicked: {
                if (__isPanning) return;
                activeClickedGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onDoubleClicked");
                mpbxGestureArea.doubleClicked(mouse);
            }

            onPressAndHold: {
                if (__isPanning) return;
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
