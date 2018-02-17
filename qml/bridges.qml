import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.2

Pane {
    id: bridges 
	 
    Label {
        text: "Look at all the bridges"
        anchors.margins: 20
        anchors.left: parent.left
        anchors.right: parent.right
        horizontalAlignment: Label.AlignHCenter
        verticalAlignment: Label.AlignVCenter
        wrapMode: Label.Wrap
    }
}
