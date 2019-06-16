import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12
import Huestacean.Types 1.0
import "."
import "MaterialDesign.js" as MD

Page {
	property int index: -1
	property var myScene: Frontend.ScenesList[index]
	property var myName: myScene.name

	function apply() {
		Frontend.PushScene(myScene, index);
	}

	Dialog {
		id: renameDialog
		modal: true
		standardButtons: Dialog.Ok

		onOpened: {
			renameText.text = myScene.name
			sizeX.value = myScene.size.x
			sizeY.value = myScene.size.y
			sizeZ.value = myScene.size.z
		}

		ColumnLayout {
			GroupBox {
				title: "Name"

				TextField {
					id: renameText
				}
			}

			GroupBox {
				title: "Size"
				ColumnLayout {
					anchors.fill: parent

					RowLayout {
						Label { 
							text: "x = "
							Layout.alignment: Qt.AlignVCenter
						}
						SpinBox {
							id: sizeX
							from: 1
							to: 30
						}
						Label { text: " m "}
					}
					

					RowLayout {
						Label { 
							text: "y = "
							Layout.alignment: Qt.AlignVCenter
						}
						SpinBox {
							id: sizeY
							from: 1
							to: 30
						}
						Label { text: " m "}
					}

					RowLayout {
						Label { 
							text: "z = "
							Layout.alignment: Qt.AlignVCenter
						}
						SpinBox {
							id: sizeZ
							from: 1
							to: 30
						}
						Label { text: " m "}
					}
				}
			}
		}
		

		onAccepted: {
			myScene.name = renameText.text
			console.log("BRADY")
			console.log(myScene.size)
			myScene.size.x = sizeX.value;
			console.log(myScene.size.x + " = " + sizeX.value)
			myScene.size.y = sizeY.value;
			console.log(myScene.size.y + " = " + sizeY.value)
			myScene.size.z = sizeZ.value;
			console.log(myScene.size.z + " = " + sizeZ.value)
			console.log(myScene.size)
			apply()
		}
	}

	header: ToolBar {
		visible: true

		RowLayout {
			anchors.fill: parent

			ToolButton {
				onClicked: {
					apply()
					Common.stack.pop()
				}

				font.pointSize: 18
				font.family: "Material Icons"
				text: MD.icons.arrow_back

				ToolTip.visible: hovered || pressed
				ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
				ToolTip.text: qsTr("Back")
			}

			ToolButton {
				onClicked: {
					renameDialog.open()
				}

				font.pointSize: 18
				font.family: "Material Icons"
				text: MD.icons.edit

				ToolTip.visible: hovered || pressed
				ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
				ToolTip.text: qsTr("Rename")
			}
				
			Label {
				font.family: "Roboto Regular"
				font.pointSize: 14

				text: myScene.name
				elide: Label.ElideRight
				verticalAlignment: Qt.AlignVCenter

				Layout.fillWidth: true
			}

			Item {
				Layout.fillWidth: true
			}


			ToolButton {
				font.pointSize: 18
				font.family: "Material Icons"
				text: MD.icons.add

				ToolTip.visible: hovered || pressed
				ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
				ToolTip.text: qsTr("Add object to scene")

				onClicked: {
					TypeFactory.AddSinePulseEffect(myScene);
					apply();
				}
			}
		}
	}

	GridLayout {
		anchors.fill: parent
		columns: 3

		ScrollView {
			id: paletteFlickable

			Layout.columnSpan: 1
			Layout.rowSpan: 3
			Layout.preferredWidth: 300
			Layout.fillHeight: true
			contentHeight: paletteColumn.height
			
			ColumnLayout {
				id: paletteColumn

				width: 300

				spacing: 0

				PaletteCategory {
					width: paletteColumn.width
					headerText: "Available Devices"

					model: Frontend.DevicesList
					delegate: PaletteItem {
						property var isInScene: {
							var isFound = false;

							for (var i = 0; i < myScene.devices.length; i++) 
							{
								if(myScene.devices[i].uniqueid == modelData.uniqueid) {
									return true;
								}
							}

							return false;
						}

						visible: !isInScene

						width: paletteColumn.width
						text: modelData.size + " " + modelData.uniqueid
						innerButtonText: "Add"
						innerButtonTooltip: "Add device to scene"
						isOddNumbered: index % 2 != 0
						onInnerButtonClicked: {
							myScene.AddDevice(modelData)
							apply();
						}
					}
				}

				PaletteCategory {
					width: paletteColumn.width
					headerText: "Effects in scene"

					model: myScene.effects
					delegate: Label {
						text: "" + modelData.data
					}
				}

				PaletteCategory {
					width: paletteColumn.width
					headerText: "Devices in scene"

					model: myScene.devicesInScene
					delegate: PaletteItem {
						width: paletteColumn.width
						text: "" + modelData.device.uniqueid
						innerButtonText: "Remove"
						innerButtonTooltip: "Remove device from scene"
						isOddNumbered: index % 2 != 0
						onInnerButtonClicked: {
							myScene.RemoveDevice(modelData.device)

							apply();
						}
					}
				}

				Item {
					Layout.fillHeight: true
				}
			}
		}

		Rectangle {
			Layout.columnSpan: 2
			Layout.rowSpan: 2

			Layout.fillWidth: true
			Layout.fillHeight: true

			Repeater {
				model: myScene.devices
				delegate: SceneItem {
					deviceIndex: index
					scene: myScene
				}
			}
		}

		Row {
			Layout.columnSpan: 2
			Layout.rowSpan: 1

			Layout.fillWidth: true

			Label {
				text: "Props will go here"
			}
		}
	}
}