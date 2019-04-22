import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.Frontend 1.0 as Frontend

import "MaterialDesign.js" as MD

ApplicationWindow {
    id: window
    width: 1000
    height: 800
    visible: true
    title: qsTr("Huestacean")

	//This doesn't want to work in qtquickcontrols2.conf
	font.family: "Roboto Regular"; 
	font.pointSize: 12;

    readonly property bool inMobileView: isMobile || window.width < window.height

	onActiveFocusItemChanged: print("activeFocusItem", activeFocusItem)
	onInMobileView: {
		if(!inMobileView) {
			drawer.close()
		}
	}

	Shortcut {
        sequences: ["Escape", "Menu"]
		context: Qt.ApplicationShortcut
        onActivated: {
			drawer.forceActiveFocus()
		}
    }
	Shortcut {
        sequences: ["Enter", "Return"]
		context: Qt.ApplicationShortcut
        onActivated: {
			Frontend.pressedEnter()
		}
    }

    Drawer {
        id: drawer

        width: 200
        height: window.height

        modal: inMobileView
        interactive: inMobileView
        position: inMobileView ? 0 : 1

        ListView {
            id: listView
            anchors.fill: parent
			focus: true

			KeyNavigation.right: stackView.currentItem

            model: ListModel {
				ListElement { title: "Home"; source: "qrc:/qml/Screensync.qml" }
				ListElement { title: "Scenes"; source: "" }
				ListElement { title: "Philips Hue"; source: "" }
				ListElement { title: "Razer Chroma"; source: "" }
				ListElement { title: "Settings"; source: "" }
				ListElement { title: "About"; source: "" }
            }

            delegate: ItemDelegate {
                    text: model.title
                    width: parent.width

                    highlighted: ListView.isCurrentItem
                    onClicked: {
                        listView.currentIndex = index
                        stack.replace(model.source)
                        if (inMobileView) {
                            drawer.close()
                        }
                    }
            }

            ScrollIndicator.vertical: ScrollIndicator { }
        }
    }

	StackView {
		id: stack
		initialItem: homeView
		anchors.fill: parent
	}
	
	Page {
		id: homeView
		header: ToolBar {
			visible: inMobileView

			RowLayout {
				anchors.fill: parent

				ToolButton {
					onClicked: drawer.visible ? drawer.close() : drawer.open()

					font.pointSize: 18
					font.family: "Material Icons"
					text: MD.icons.menu
				}

				ToolButton {
					onClicked: drawer.visible ? drawer.close() : drawer.open()

					font.pointSize: 18
					font.family: "Material Icons"
					text: MD.icons.arrow_back
				}
				
				Label {
					font.family: "Roboto Bold"
					font.pointSize: 14

					text: "Huestacean"
					elide: Label.ElideRight
					verticalAlignment: Qt.AlignVCenter

					Layout.fillWidth: true
				}
				
			}
		}

		Label {
			text: 'Huestacean on GitHub: <a href="https://github.com/BradyBrenot/huestacean">https://github.com/BradyBrenot/huestacean</a>'
			anchors.margins: 20
			wrapMode: Label.Wrap

			onLinkActivated: Qt.openUrlExternally(link)
		}
	}
}