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

        anchors.fill: parent

        //! Holds previous zoom level value
        property double __oldZoom

        //! Animation properties
        property var    __startTime
        property var    __lastUpdateTime
        property double __rate
        property double __reference
        property double __lastUpdateValue
        property double __animationZoom: 0
        property var    __animationCenter

        readonly property int    __thresholdTime: 100 // in ms
        readonly property double __thresholdRate: 0.1 // zoom units per second
        readonly property double __deceleration: 20 // zoom units per second
        readonly property double __maxDuration: 3e3 // in ms
        readonly property double __maxChange: 4 // in zoom units

        //! Calculate zoom level
        function calcZoomDelta(zoom, percent) {
            return zoom + Math.log(percent)/Math.log(2)
        }

        function calcRate(zoom) {
            var t = new Date().getTime();
            var eL = t - __lastUpdateTime;
            var eS = t - __startTime;
            if (eL > __thresholdTime) {
                __rate = (zoom - __lastUpdateValue) / eL * 1e3;
                __startTime = __lastUpdateTime + eL;
                __reference = zoom;
            } else if (eS > __thresholdTime) {
                __rate = (zoom - __reference) / eS * 1e3;
                __startTime = __startTime + eS;
                __reference = zoom;
            }
        }


        NumberAnimation {
            id: pinchAnim
            target: pincharea
            property: "__animationZoom"
            easing.type: Easing.OutQuad
        }

        on__AnimationZoomChanged: map.setZoomLevel(__animationZoom, __animationCenter)

        //! Save previous zoom level when pinch gesture started
        onPinchStarted: {
            pinchAnim.stop()
            __oldZoom = map.zoomLevel;
            __startTime = new Date().getTime();
            __rate = 0;
            __reference = __oldZoom;
            __lastUpdateValue = __oldZoom;
            __lastUpdateTime = __startTime;
        }

        //! Update map's zoom level when pinch is updating
        onPinchUpdated: {
            var newZoom = calcZoomDelta(__oldZoom, pinch.scale)
            map.setZoomLevel(newZoom, pinch.center)
            calcRate(newZoom);
            __lastUpdateValue = newZoom;
            __lastUpdateTime = new Date().getTime();
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

            // Start animation if pinch speed was fast towards the end
            if (Math.abs(__rate) > __thresholdRate) {
                __oldZoom = newZoom;
                __animationCenter = pinch.center;
                __animationZoom = newZoom;
                pinchAnim.duration = Math.min(Math.abs(__rate) / __deceleration * 1e3, __maxDuration)
                pinchAnim.from = newZoom;
                pinchAnim.to = Math.max(map.minimumZoomLevel, __oldZoom - __maxChange,
                                        Math.min(newZoom + __rate*pinchAnim.duration/1e3,
                                                 __oldZoom + __maxChange, map.maximumZoomLevel))
                //console.log('Zoom animation ' + pinchAnim.duration + ' ' + pinchAnim.from + ' ' + pinchAnim.to + ' ' + (newZoom + __rate*pinchAnim.duration/1e3))
                pinchAnim.start()
            }
        }

        Flickable {
            id: flick

            anchors.fill: parent
            contentHeight: height*40
            contentWidth: width*40
            contentX: contentWidth/2 - width/2
            contentY: contentHeight/2 - height/2

            property int  __lastX: 0
            property int  __lastY: 0

            onMovementEnded: {
                contentX = contentWidth/2
                contentY = contentHeight/2
            }

            onMovementStarted: {
                __lastX = contentX
                __lastY = contentY
            }

            onContentXChanged: update()
            onContentYChanged: update()

            function update() {
                if (!moving) return
                var dx = __lastX - contentX
                var dy = __lastY - contentY
                map.pan(dx, dy)
                __lastX = contentX
                __lastY = contentY
            }

            MouseArea {
                id: mousearea

                height: mpbxGestureArea.height
                width: mpbxGestureArea.width

                x: flick.contentX
                y: flick.contentY

                property var constants: QtObject {
                    property string eventPrefix: "MAPBOX MAP GESTURE AREA - "
                }

                onWheel: {
                    map.setZoomLevel( map.zoomLevel + 0.2 * wheel.angleDelta.y / 120, Qt.point(wheel.x, wheel.y) )
                }

                onPressed: {
                    if (pinchAnim.running) pinchAnim.stop();
                }

                /////////////////////////////////////////////////////////
                /// exported signals

                onClicked: {
                    activeClickedGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onClicked");
                    mpbxGestureArea.clicked(mouse);
                }

                onDoubleClicked: {
                    activeClickedGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onDoubleClicked");
                    mpbxGestureArea.doubleClicked(mouse);
                }

                onPressAndHold: {
                    activePressAndHoldGeo && map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), constants.eventPrefix + "onPressAndHold");
                    mpbxGestureArea.pressAndHold(mouse);
                }

                onReleased: {
                    mpbxGestureArea.released(mouse);
                }
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

    Component.onCompleted: {
        map.gestureInProgress = Qt.binding(function () {
            return flick.moving || pinchAnim.running || pincharea.pinch.active;
        });
    }
}
