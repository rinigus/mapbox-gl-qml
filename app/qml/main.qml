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

        styleUrl: "mapbox://styles/mapbox/streets-v10"

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

            onClicked: {
//                var routeSource = {
//                    "type": "geojson",
//                    "data": '{
//                        "type": "Feature",
//                        "properties": {},
//                        "geometry": {
//                            "type": "LineString",
//                            "coordinates": [
//                                [24.942046, 60.170448],
//                                [
//                                    24.934420000000003,
//                                    60.163500000000006
//                                ],
//                                [
//                                    24.923490008921945,
//                                    60.16159500239787
//                                ],
//                                [
//                                    24.916150000000002,
//                                    60.171530000000004
//                                ],
//                                [
//                                    24.931620000000002,
//                                    60.18218
//                                ],
//                                [
//                                    24.961660000000002,
//                                    60.17557000000001
//                                ],
//                                [
//                                    24.954860000000004,
//                                    60.158930000000005
//                                ],
//                                [
//                                    24.943690000000004,
//                                    60.155280000000005
//                                ]
//                            ]
//                        }
//                    }'
//                }

//                var layer_id = "routeCase"
//                var layer = { "type": "line", "source": "route" }

//                map.addSource("route", routeSource)
//                map.addLayer(layer_id, layer)

//                map.setLayoutProperty("routeCase", "line-join", "round");
//                map.setLayoutProperty("routeCase", "line-cap", "round");
//                map.setPaintProperty("routeCase", "line-color", "white");
//                map.setPaintProperty("routeCase", "line-width", 20.0);
//                console.log("Clicked")
            }
        }

        Component.onCompleted: {

            //            // List default styles
            //            var styles = map.defaultStyles()
            //            for (var i=0; i<styles.length; i++)
            //            {
            //                var o = styles[i];
            //                console.log(o["name"] + " -> " + o["url"])
            //            }

            // map.setMargins(0, 0.5, 0, 0);

            var routeSource = {
                "type": "geojson",
                "data": '{
                    "type": "Feature",
                    "properties": {},
                    "geometry": {
                        "type": "LineString",
                        "coordinates": [
                            [24.942046, 60.170448],
                            [
                                24.934420000000003,
                                60.163500000000006
                            ],
                            [
                                24.923490008921945,
                                60.16159500239787
                            ],
                            [
                                24.916150000000002,
                                60.171530000000004
                            ],
                            [
                                24.931620000000002,
                                60.18218
                            ],
                            [
                                24.961660000000002,
                                60.17557000000001
                            ],
                            [
                                24.954860000000004,
                                60.158930000000005
                            ],
                            [
                                24.943690000000004,
                                60.155280000000005
                            ]
                        ]
                    }
                }'
            }

            var layer_id = "routeCase"
            var layer = { "type": "line", "source": "route" }

            map.addSource("route", routeSource)
            map.addLayer(layer_id, layer, "waterway-label")

            map.setLayoutProperty("routeCase", "line-join", "round");
            map.setLayoutProperty("routeCase", "line-cap", "round");
            map.setPaintProperty("routeCase", "line-color", "white");
            map.setPaintProperty("routeCase", "line-width", 20.0);
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
