import QtQuick 2.0
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtPositioning 5.3

import MapboxMap 1.0

import "."

ApplicationWindow {
    id: appWindow

    title: "Mapbox GL QML example"
    width: 1024
    height: 768
    visible: true

    MapboxMap {
        id: map
        anchors.fill: parent

        center: QtPositioning.coordinate(59.436962, 24.753574) // Tallinn
        //center: QtPositioning.coordinate(60.16, 24.94) // Helsinki
        zoomLevel: 12.0
        metersPerPixelTolerance: 0.1
        minimumZoomLevel: 0
        maximumZoomLevel: 20
        pixelRatio: 1.0
        useFBO: true

        bearing: bearingSlider.value
        pitch: pitchSlider.value

        cacheDatabaseStoreSettings: true
        //cacheDatabaseDefaultPath: true

        //cacheDatabaseMaximalSize: 20*1024*1024
        //cacheDatabasePath: "/tmp/mapbox/mbgl-cache.db"
        cacheDatabasePath: ":memory:"

        Behavior on center {
            CoordinateAnimation {
                duration: 500
                easing.type: Easing.InOutQuad
            }
        }

        Behavior on margins {
            PropertyAnimation {
                duration:500
                easing.type: Easing.InOutQuad
            }
        }

//        Behavior on margins.height { NumberAnimation { duration: 500 } }
//        Behavior on margins.y { NumberAnimation { duration: 500 } }
//        Behavior on margins.x { NumberAnimation { duration: 500 } }
//        Behavior on margins.width { NumberAnimation { duration: 500 } }

//        styleJson: '
//        {
//            "sources": {
//                "raster": {
//                    "tiles": ["http://localhost:8553/v1/tile?scale=2&z={z}&x={x}&y={y}"],
//                    "type": "raster",
//                    "tileSize": 512
//                }
//            },
//            "layers": [
//                {
//                    "id": "raster",
//                    "type": "raster",
//                    "source": "raster",
//                    "layout": {
//                        "visibility": "visible"
//                    },
//                    "paint": {
//                        "raster-opacity": 1
//                    }
//                }
//            ],
//            "id": "raster"
//        }'

        //accessToken: "INSERT_THE_TOKEN_OR_DEFINE_IN_ENVIRONMENT"

        property string styleUrlOrig: "http://localhost:8553/v1/mbgl/style?style=osmbright"
        property int styleIndex: 0

        //styleUrl: "mapbox://styles/mapbox/outdoors-v10" //"mapbox://styles/mapbox/streets-v10"
        styleUrl: styleUrlOrig

        urlDebug: true

        MapboxMapGestureArea {
            id: mouseArea
            map: map
            activeClickedGeo: true
            activeDoubleClickedGeo: true
            activePressAndHoldGeo: true

            onClicked: {
                console.log("Click: " + mouse)
                map.queryCoordinateForPixel(Qt.point(mouse.x, mouse.y), "pin_press")
            }
            onDoubleClicked: {
                map.setZoomLevel(map.zoomLevel + 1, Qt.point(mouse.x, mouse.y) );
//                //map.center = QtPositioning.coordinate(60.170448, 24.942046)
//                map.margins = Qt.rect(
//                    0.5,               // x
//                    0.05,   // y
//                    0.5,                // width
//                    0.2 // height
//                );

                console.log("Double click: " + mouse)
            }
            onPressAndHold: console.log("Press and hold: " + mouse)

            onClickedGeo: console.log("Click geo: " + geocoordinate + " sensitivity: " + degLatPerPixel + " " + degLonPerPixel)
            onDoubleClickedGeo: console.log("Double click geo: " + geocoordinate + " sensitivity: " + degLatPerPixel + " " + degLonPerPixel)
            onPressAndHoldGeo: {
                console.log("Press and hold geo: " + geocoordinate + " sensitivity: " + degLatPerPixel + " " + degLonPerPixel);

                var tname = "track-" + geocoordinate
                map.trackLocation(tname, geocoordinate);
                var component = Qt.createComponent("location.qml")
                component.createObject(appWindow, {"trackname": tname})
            }
        }

        onStyleJsonChanged: {
            console.log("Swapping languages")
            var ns = styleJson.replace("{name_en}", "{name}")
            styleJson = ns;
        }

        Component.onCompleted: {

            // List default styles
            var styles = map.defaultStyles()
            for (var i=0; i<styles.length; i++)
            {
                var o = styles[i];
                console.log(o["name"] + " -> " + o["url"])
            }

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

            map.addLayer("routeCase", { "type": "line", "source": "route" }, "waterway-name")
            map.setLayoutProperty("routeCase", "line-join", "round");
            map.setLayoutProperty("routeCase", "line-cap", "round");
            map.setPaintProperty("routeCase", "line-color", "white");
            map.setPaintProperty("routeCase", "line-width", 15.0);

            map.addLayer("route", { "type": "line", "source": "route" }, "waterway-name")
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

            map.addLayer("location-uncertainty", {"type": "circle", "source": "location"}, "waterway-name")
            map.setPaintProperty("location-uncertainty", "circle-radius", 20)
            map.setPaintProperty("location-uncertainty", "circle-color", "#87cefa")
            map.setPaintProperty("location-uncertainty", "circle-opacity", 0.25)

            map.addLayer("location-case", {"type": "circle", "source": "location"}, "waterway-name")
            map.setPaintProperty("location-case", "circle-radius", 10)
            map.setPaintProperty("location-case", "circle-color", "white")

            map.addLayer("location", {"type": "circle", "source": "location"})
            map.setPaintProperty("location", "circle-radius", 5)
            map.setPaintProperty("location", "circle-color", "blue")

            map.addLayer("location-label", {"type": "symbol", "source": "location"})
            map.setLayoutProperty("location-label", "text-allow-overlap", true)
            map.setLayoutProperty("location-label", "icon-allow-overlap", true)
            map.setLayoutProperty("location-label", "icon-image", "position-icon")

            //map.setLayoutProperty("location-label", "text-ignore-placement", true)
            map.setLayoutProperty("location-label", "text-field", "{name}")
            map.setLayoutProperty("location-label", "text-justify", "left")
            map.setLayoutProperty("location-label", "text-anchor", "top-left")
            //map.setLayoutPropertyList("location-label", "text-offset", [0.2, 0.2])
            //map.setLayoutPropertyList("location-label", "text-font", ["Klokantech Noto Sans Regular"])
            map.setPaintProperty("location-label", "text-halo-color", "white")
            map.setPaintProperty("location-label", "text-halo-width", 2)


            /// multipoint source
            var points = [
                        QtPositioning.coordinate(60.15, 24.93),
                        QtPositioning.coordinate(60.17, 24.95),
                        QtPositioning.coordinate(60.18, 24.96)
                    ]
            var names = [ "P1", "P2", "P3" ]
            map.addSourcePoints("points", points, names)

            var params = {
                "type": "circle",
                "source": "points",
                "filter": ["==", "name", "P3"],
            };

            map.addLayer("points-centers", params)
            map.setPaintProperty("points-centers", "circle-radius", 5)
            map.setPaintProperty("points-centers", "circle-color", "blue")

            map.addLayer("points-label", {"type": "symbol", "source": "points"})
            map.setLayoutProperty("points-label", "text-field", "{name}")
            map.setLayoutProperty("points-label", "text-justify", "left")
            map.setLayoutProperty("points-label", "text-anchor", "top-left")
            map.setLayoutPropertyList("points-label", "text-offset", [0.2, 0.2])
            map.setPaintProperty("points-label", "text-halo-color", "green")
            map.setPaintProperty("points-label", "text-halo-width", 2)

//            /// road as line source
//            var line = [
//                        QtPositioning.coordinate(60.16, 24.94),
//                        QtPositioning.coordinate(60.17, 24.93),
//                        QtPositioning.coordinate(60.17, 24.92)
//                    ]
//            map.addSourceLine("linesrc", line, "Line")

//            map.addLayer("line", { "type": "line", "source": "linesrc" })
//            map.setLayoutProperty("line", "line-join", "round");
//            map.setLayoutProperty("line", "line-cap", "round");
//            map.setPaintProperty("line", "line-color", "green");
//            map.setPaintProperty("line", "line-width", 10.0);

//            //map.fitView(line);

//            /// track location
//            map.trackLocation("track-1", QtPositioning.coordinate(60.16, 24.94));

//            console.log("Margins: " + map.margins)
        }

        Connections {
            target: map
            onReplySourceExists: {
                console.log("Source: " + id + " " + exists)
            }

            onReplyLayerExists: console.log("Layer: " + id + " " + exists)

            onErrorChanged: console.log("Error message: " + error)

            onLocationChanged: console.log("Location: " + id + " " + visible + " " + pixel)

            onReplyCoordinateForPixel: {
                console.log("Listen to all replies for coordinates: " + tag + " " + geocoordinate)
                console.log("lat: "+ geocoordinate.latitude)
                console.log("lng: "+ geocoordinate.longitude)
            }

        }

        focus: true
        Keys.onPressed: {
            if (event.key === Qt.Key_R) {
                var src = map.styleUrlOrig;
                map.styleIndex += 1;
                map.styleUrl = src + "&index=" + String(map.styleIndex);
                console.log("Reloaded");
            }
            if (event.key === Qt.Key_C) {
                map.clearCache();
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

    Label {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        text: "Scale: %0 m/pixel ; zoom: %1".arg(map.metersPerPixel.toFixed(2)).arg(map.zoomLevel)
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

//    Timer {
//        property double angle: 0.0

//        interval: 10
//        running: true
//        repeat: true
//        onTriggered: {
//            angle += 1.0 / 180. * Math.PI
//            if (angle > Math.PI*2)
//                angle -= Math.PI*2

//            map.updateSourcePoint("location",
//                                  QtPositioning.coordinate(60.16 +  0.01*Math.sin(angle), 24.94 + 0.01*Math.cos(angle)),
//                                  "hello, my angle is " + (angle/Math.PI*180).toFixed(1));
//            //map.center = QtPositioning.coordinate(60.16 +  0.01*Math.sin(angle), 24.94 + 0.01*Math.cos(angle))
//        }
//    }

//    Timer {
//        interval: 3000
//        running: true
//        onTriggered: {
//            console.log(map.margins)
//            map.margins = Qt.rect(0.1, 0.01, 0.8, 0.4)
//            console.log("Margins changed: " + map.margins)
//        }
//    }

//        Timer {
//            interval: 3000
//            running: true
//            onTriggered: {
//                map.urlDebug = true
//                map.styleUrl = "mapbox://styles/mapbox/traffic-night-v2"
//            }
//        }
}
