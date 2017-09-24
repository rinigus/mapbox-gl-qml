import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtPositioning 5.3

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
                //                console.log("Clicked")
                //                map.querySourceExists("route");
                //                map.querySourceExists("route-shouldnt-be-there");
                //                map.queryLayerExists("routeCase");
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

            map.addSource("route", routeSource)

            map.addLayer("routeCase", { "type": "line", "source": "route" }, "waterway-label")
            map.setLayoutProperty("routeCase", "line-join", "round");
            map.setLayoutProperty("routeCase", "line-cap", "round");
            map.setPaintProperty("routeCase", "line-color", "white");
            map.setPaintProperty("routeCase", "line-width", 15.0);

            map.addLayer("route", { "type": "line", "source": "route" }, "waterway-label")
            map.setLayoutProperty("route", "line-join", "round");
            map.setLayoutProperty("route", "line-cap", "round");
            map.setPaintProperty("route", "line-color", "blue");
            map.setPaintProperty("route", "line-width", 10.0);
            map.setPaintPropertyList("route", "line-dasharray", [1,2]);

            /// Location support
            map.addSource("location",
                          {"type": "geojson",
                              "data": {
                                  "type": "Feature",
                                  "properties": { "name": "location" },
                                  "geometry": {
                                      "type": "Point",
                                      "coordinates": [
                                          (24.94),
                                          (60.16)
                                      ]
                                  }
                              }
                          })

            map.addLayer("location-uncertainty", {"type": "circle", "source": "location"}, "waterway-label")
            map.setPaintProperty("location-uncertainty", "circle-radius", 20)
            map.setPaintProperty("location-uncertainty", "circle-color", "#87cefa")
            map.setPaintProperty("location-uncertainty", "circle-opacity", 0.25)

            map.addLayer("location-case", {"type": "circle", "source": "location"}, "waterway-label")
            map.setPaintProperty("location-case", "circle-radius", 10)
            map.setPaintProperty("location-case", "circle-color", "white")

            map.addLayer("location", {"type": "circle", "source": "location"}, "waterway-label")
            map.setPaintProperty("location", "circle-radius", 5)
            map.setPaintProperty("location", "circle-color", "blue")

            map.addLayer("location-label", {"type": "symbol", "source": "location"})
            map.setLayoutProperty("location-label", "text-field", "{name}")
            map.setLayoutProperty("location-label", "text-justify", "left")
            map.setLayoutProperty("location-label", "text-anchor", "top-left")
            map.setLayoutPropertyList("location-label", "text-offset", [0.2, 0.2])
            map.setPaintProperty("location-label", "text-halo-color", "white")
            map.setPaintProperty("location-label", "text-halo-width", 2)

        }

        Connections {
            target: map
            onReplySourceExists: {
                console.log("Source: " + id + " " + exists)
            }

            onReplyLayerExists: console.log("Layer: " + id + " " + exists)

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

    Label {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        text: "Scale: %0 m/pixel".arg(map.metersPerPixel.toFixed(2))
    }

    //    PositionSource {
    //        id: gps

    //        active: true
    //        updateInterval: 1000

    //        function update() {
    //            if (gps.position.longitudeValid && gps.position.latitudeValid)
    //            {
    //                map.updateSource("location", {"type": "geojson", "data":
    //                                  '{
    //                                      "type": "Feature",
    //                                      "properties": {},
    //                                      "geometry": {
    //                                        "type": "Point",
    //                                        "coordinates": [' +
    //                                     gps.position.coordinate.longitude + ', ' +
    //                                     gps.position.coordinate.latitude + '
    //                                        ]
    //                                      }
    //                                    }'
    //                                  } )
    //                console.log("Location updated")
    //            }

    //            if (gps.position.horizontalAccuracyValid && gps.position.verticalAccuracyValid)
    //            {
    //                var accuracy = Math.max(gps.position.horizontalAccuracy, gps.position.verticalAccuracy)
    //                map.setPaintProperty("location-uncertainty", "circle-radius", accuracy / map.metersPerPixel)
    //                console.log("Uncertainty updated")
    //            }

    //            if (gps.position.directionValid)
    //                map.bearing = gps.position.direction
    //        }


    //        onPositionChanged: {
    //            update()
    //        }

    //        Component.onCompleted: {
    //            update()
    //        }
    //    }

    //    Connections {
    //        target: map
    //        onMetersPerPixelChanged: gps.update()
    //    }

    Timer {
        property double angle: 0.0

        interval: 25
        running: true
        repeat: true
        onTriggered: {
            angle += 1.0 / 180. * Math.PI
            if (angle > Math.PI*2)
                angle -= Math.PI*2

            map.updateSourcePoint("location",
                                  QtPositioning.coordinate(60.16 +  0.01*Math.sin(angle), 24.94 + 0.01*Math.cos(angle)),
                                  "hello, my angle is " + (angle/Math.PI*180).toFixed(1));
        }
    }

}
