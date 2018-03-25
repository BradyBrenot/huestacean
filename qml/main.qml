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

	onActiveFocusItemChanged: print("activeFocusItem", activeFocusItem)

	Shortcut {
        sequences: ["Escape", "Menu"]
		context: Qt.ApplicationShortcut
        onActivated: {
			print("f you")
			drawer.forceActiveFocus()
		}
    }

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
			focus: true

			KeyNavigation.right: stackView.currentItem

            model: ListModel {
				ListElement { title: "Bridges"; source: "qrc:/qml/bridges.qml" }
                ListElement { title: "Screen Sync"; source: "qrc:/qml/screensync.qml" }
				ListElement { title: "About"; source: "qrc:/qml/about.qml" }
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

	Flickable {
		id: flick

		KeyNavigation.up: overlayHeader
		KeyNavigation.left: drawer
		
		anchors.fill: parent
		anchors.topMargin: (inPortrait ? overlayHeader.height : 0)
		anchors.leftMargin: !inPortrait ? drawer.width : undefined
		anchors.rightMargin: 0
		anchors.left: parent.left
		anchors.right: parent.right

		contentHeight: stackView.currentItem.implicitHeight
		contentWidth: Math.max(stackView.currentItem.implicitWidth, width)


		StackView {
			id: stackView

			topPadding: 20
			bottomPadding: 20

			initialItem: "qrc:/qml/bridges.qml"
		}

		ScrollBar.vertical: ScrollBar { 
			contentItem.opacity: flick.contentHeight > flick.height ? 1 : 0;
		} 

		ScrollBar.horizontal: ScrollBar { 
			contentItem.opacity: flick.contentWidth > flick.width ? 1 : 0;
		} 
	}
}
