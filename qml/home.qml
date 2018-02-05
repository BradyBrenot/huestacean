import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.2
import Huestacean 1.0

Pane {
    id: home

    Column {
        id: column
        width: 200
        height: 400

        Label {
            text: Huestacean.hue.message
            anchors.margins: 20
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Label.AlignHCenter
            verticalAlignment: Label.AlignVCenter
            wrapMode: Label.Wrap
        }

        Button {
            text: qsTr("Connect!")
            onClicked: Huestacean.hueBridge.connectToBridge()
        }

        Button {
            text: qsTr("Entertain me!")
            onClicked: Huestacean.hueBridge.testEntertainment()
        }

        Button {
            text: qsTr("Show groups!")
            onClicked: Huestacean.hueBridge.requestGroups()
        }

        Button {
            text: qsTr("Ssdp search")
            onClicked: Huestacean.bridgeDiscovery.startSearch()
        }
    }
}
