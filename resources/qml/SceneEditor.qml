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

		TextField {
			id: renameText
			placeholderText: myScene.name
		}

		onAccepted: {
			myScene.name = renameText.text
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
				onClicked: renameDialog.open()

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
					var newList = myScene.effects;
					newList.push(TypeFactory.NewConstantEffect());
					myScene.effects = newList;
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
						width: paletteColumn.width
						text: "" + modelData.data
						innerButtonText: "Add"
						innerButtonTooltip: "Add device to scene"
						isOddNumbered: index % 2 != 0
					}
				}

				PaletteCategory {
					width: paletteColumn.width
					headerText: "Effects In Scene"

					model: myScene.effects
					delegate: Label {
						text: "" + modelData.data
					}
				}

				PaletteCategory {
					width: paletteColumn.width
					headerText: "Effects In Scene"

					model: myScene.devicesInScene
					delegate: Label {
						text: "" + modelData
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