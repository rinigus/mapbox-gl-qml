import QtQuick 2.0
import QtQuick.Controls 1.0

import QQuickItemMapboxGL 1.0

ApplicationWindow {
    title: "Mapbox GL QML example"
    width: 1024
    height: 768
    visible: true

    MapboxMap {
        id: map
        anchors.fill: parent
    }
}
