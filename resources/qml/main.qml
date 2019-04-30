import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12

import "."

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

    readonly property bool inMobileView: (isMobile != undefined && isMobile) || window.width < window.height

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
			GuiHelper.pressedEnter()
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

			KeyNavigation.right: Common.stack.currentItem

            model: ListModel {
				ListElement { title: "Home"; source: "" }
				ListElement { title: "Scenes"; source: "qrc:/qml/SceneListScreen.qml" }
				ListElement { title: "Philips Hue"; source: "qrc:/qml/Hue.qml" }
				ListElement { title: "Razer Chroma"; source: "qrc:/qml/Razer.qml" }
				ListElement { title: "Settings"; source: "qrc:/qml/Settings.qml" }
				ListElement { title: "About"; source: "qrc:/qml/About.qml" }
            }

			function onClickedBack(person, notice) {
				Common.stack.pop()
			}

            delegate: ItemDelegate {
                text: model.title
                width: parent.width

                highlighted: false
                onClicked: {
                    listView.currentIndex = index
					Common.stack.pop(Common.stack.initialItem)
					if("" != model.source) {
						Common.stack.push(model.source)
					}
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

		Component.onCompleted: Common.stack = stack
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

					ToolTip.visible: hovered || pressed
					ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
					ToolTip.text: qsTr("Menu")
				}
				
				Label {
					font.family: "Roboto Regular"
					font.pointSize: 14

					text: "Huestacean"
					elide: Label.ElideRight
					verticalAlignment: Qt.AlignVCenter

					Layout.fillWidth: true
				}
			}
		}

		GridLayout {
			id: grid
			columns: inMobileView ? 1 : 2
			anchors.fill: parent
			anchors.margins: inMobileView ? 0 : 60
			anchors.leftMargin: inMobileView ? 0 : 80
			anchors.rightMargin: inMobileView ? 0 : 80
			columnSpacing: inMobileView ? 0 : 40
			rowSpacing: inMobileView ? 0 : 30

			SceneList {
			
			}

			Pane {
				visible: false

				Layout.fillWidth: true
				Material.background: Material.color(Material.Blue, Material.Shade600)

				RowLayout {
					anchors.fill: parent

					Column {
						Label {
							text: "Last scene" 
							font.pointSize: 18
						}
						Label {
							text: "" + Frontend.DevicesList[0] 
							font.pointSize: 12
						}
					}
					
					Switch {
						Layout.alignment: Qt.AlignRight
					}
				}
									
			}

			Button {
				id: hueButton
				padding: 30
				Layout.columnSpan: 1
				Layout.fillWidth: true
				Layout.preferredWidth: grid.width / grid.columns

				//Material.background: Material.color(Material.Blue, Material.Shade900)

				contentItem: Column {
					Label { text: "Phlips Hue"; font.pointSize: 18; }
					Label { text: "Not detected!"; font.pointSize: 12; }
				}

				background.anchors.fill: hueButton
			}

			Button {
				id: razerButton
				padding: 30
				Layout.columnSpan: 1
				Layout.fillWidth: true
				Layout.preferredWidth: grid.width / grid.columns

				//Material.background: Material.color(Material.Green, Material.Shade900)

				contentItem: Column {
					Label { text: "Razer Chroma"; font.pointSize: 18; }
					Label { text: "Not detected!"; font.pointSize: 12; }
				}

				background.anchors.fill: razerButton
			}

			Button {
				id: remoteButton
				padding: 30
				Layout.columnSpan: 1
				Layout.fillWidth: true
				Layout.preferredWidth: grid.width / grid.columns

				//Material.background: Material.color(Material.Teal, Material.Shade900)

				contentItem: Column {
					Label { text: "Remote control"; font.pointSize: 18; }
					Label { text: "Not enabled"; font.pointSize: 12; }
				}

				background.anchors.fill: remoteButton
			}

			Button {
				id: settingsButton
				padding: 30
				Layout.columnSpan: 1
				Layout.fillWidth: true
				Layout.preferredWidth: grid.width / grid.columns

				//Material.background: Material.color(Material.Red, Material.Shade900)

				contentItem: Column {
					Label { text: "Settings"; font.pointSize: 18; }
					Label { text: "Global settings and About Huestacean"; font.pointSize: 12; wrapMode: Text.WordWrap; }
				}

				background.anchors.fill: settingsButton
			}

			Item {
				Layout.fillHeight: true
			}
		}
	}

	
}