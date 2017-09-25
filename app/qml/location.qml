import QtQuick 2.0
import QtQuick.Controls 1.4

Rectangle {
    property string trackname: ""

    id: main

    width: content.width + 10
    height: content.height + 10

    x: 0
    y: 0
    color: "white"

    Column {
        id: content
        anchors.centerIn: parent

        Label {
            text: "Tracker: " + trackname
        }

        Button {
            anchors.right: parent.right
            anchors.left: parent.left

            text: "Remove me"

            onClicked: {
                map.removeLocationTracking(trackname)
            }
        }
    }

    Connections {
        target: map

        onLocationChanged: {
            if (id !== trackname) return;

            x = pixel.x
            y = pixel.y
            main.visible = visible
        }

        onLocationTrackingRemoved: {
            if (id !== trackname) return;
            main.destroy()
        }
    }
}
