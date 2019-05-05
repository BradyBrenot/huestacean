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

		Column {
			Layout.columnSpan: 1
			Layout.rowSpan: 3

			Column {
				Label {
					text: "In Scene"
				}

				Repeater {
					model: myScene.effects
					Label {
						text: "" + modelData
					}
				}

				Repeater {
					model: myScene.devicesInScene
					Label {
						text: "" + modelData
					}
				}

				Label {
					text: "Available"
				}

				Label {
					text: "Sine Pulse"
				}

				Label {
					text: "Constant Pulse"
				}

				Repeater {
					model: Frontend.DevicesList
					Label {
						text: "" + modelData
					}
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