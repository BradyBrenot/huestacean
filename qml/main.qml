import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.2

ApplicationWindow {
    id: window
    width: 1000
    height: 800
    visible: true
    title: qsTr("Huestacean")

    readonly property bool inPortrait: window.width < window.height

    ToolBar {
        id: overlayHeader
        width: parent.width
        parent: window.overlay
        visible: inPortrait

        Label {
            id: label
            color: "white"
            anchors.centerIn: parent
            text: "Huestacean"
            font.pointSize: 16
            font.bold: true
        }


        ToolButton {
            id: button
            y: 0
            width: 58
            height: 48
            display: AbstractButton.IconOnly
            anchors.left: parent.left
            anchors.leftMargin: 13
            onClicked: drawer.visible ? drawer.close() : drawer.open()
            visible: inPortrait

            icon.source: "qrc:/images/hamburger.png"
        }
    }

    Drawer {
        id: drawer

        width: 200
        height: window.height

        modal: inPortrait
        interactive: inPortrait
        position: inPortrait ? 0 : 1
        visible: !inPortrait

        ListView {
            id: listView
            anchors.fill: parent

            footer: ItemDelegate {
                id: footer
                text: qsTr("Footer")
                width: parent.width

                MenuSeparator {
                    parent: footer
                    width: parent.width
                    anchors.verticalCenter: parent.top
                }
            }

            model: ListModel {
                ListElement { title: "Home"; source: "qrc:/qml/home.qml" }
                ListElement { title: "Bridges"; source: "qrc:/qml/bridges.qml" }
            }

            delegate: ItemDelegate {
                    text: model.title
                    width: parent.width

                    highlighted: ListView.isCurrentItem
                    onClicked: {
                        listView.currentIndex = index
                        stackView.replace(model.source)
                        if (inPortrait) {
                            drawer.close()
                        }
                    }
            }

            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

    StackView {
        id: stackView

        anchors.fill: parent
        anchors.topMargin: (inPortrait ? overlayHeader.height : 0)
        anchors.leftMargin: !inPortrait ? drawer.width : undefined

        topPadding: 20
        bottomPadding: 20

        initialItem: "qrc:/qml/home.qml"
    }
}
