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

				ToolButton {
					onClicked: drawer.visible ? drawer.close() : drawer.open()

					font.pointSize: 18
					font.family: "Material Icons"
					text: MD.icons.arrow_back

					ToolTip.visible: hovered || pressed
					ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
					ToolTip.text: qsTr("Back")
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

			Pane {
				//visible: !inMobileView
				Material.elevation: 4
				Layout.columnSpan: inMobileView ? 1 : 2
				Layout.fillHeight: true
				Layout.fillWidth: true
				Layout.minimumHeight: inMobileView ? 100 : 200

				Material.background: Material.color(Material.Grey, Material.Shade800)
				padding: inMobileView ? 0 : 12

				ColumnLayout {
					anchors.fill: parent

					Label {
						visible: !inMobileView
						font.family: "Roboto Regular"
						font.pointSize: 18

						text: "Scenes"
					}

					Flickable {
						id: scenesFlick
						clip: true

						Layout.fillHeight: true
						Layout.fillWidth: true
						
						ColumnLayout {
							id: scenesColumn
							anchors.left: parent.left
							anchors.right: parent.right
							spacing: 4

							Repeater {
								model: Frontend.DevicesList
								Button {
									id: sceneButton
									padding: 10
									Material.elevation: 6
									Material.background: Material.color(Material.Blue, Material.Shade600)
									Layout.fillWidth: true

									background: Item {
										anchors.fill: parent
										Rectangle {
											id:rect
											anchors.fill:parent
											radius: 6
										}

										LinearGradient {
											anchors.fill: parent
											start: Qt.point(0, 0)
											end: Qt.point(sceneButton.width, 0)
											source: rect
											gradient: Gradient {
												GradientStop { position: 0.0; color: Material.color(Material.Blue, Material.Shade200) }
												GradientStop { position: 0.5; color: Material.color(Material.Blue, Material.Shade600) }
												GradientStop { position: 1.0; color: Material.color(Material.Green, Material.Shade600) }
											}
										}
									}

									contentItem: RowLayout {
										id: rowLayout

										Item {
											height: label.height
											width: label.width

											Label {
												color: "black"
												text: "" + modelData
												x: label.x + 1
												y: label.y + 1
												opacity: 0.6
											}

											Label {
												anchors.verticalCenter: parent.verticalCenter
												id: label
												text: "" + modelData 
											}
										}
										
										Switch {
											Layout.alignment: Qt.AlignRight
										}
									}
								}
							}
						}

						contentHeight: scenesColumn.height
						contentWidth: width

						ScrollBar.vertical: ScrollBar { 
							contentItem.opacity: scenesFlick.contentHeight > scenesFlick.height ? 1 : 0;
						} 

						ScrollBar.horizontal: ScrollBar { 
							contentItem.opacity: scenesFlick.contentWidth > scenesFlick.width ? 1 : 0;
						} 
					}

					Button {
						Layout.alignment: Qt.AlignRight
						Layout.margins: 4
						text: "New scene"
					}
				}
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
		}
	}

	
}