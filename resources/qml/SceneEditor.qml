import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12

import "."
import "MaterialDesign.js" as MD

Page {
	property int index: -1
	property var myScene: Frontend.ScenesList[index]
	property var myName: myScene.name

	function apply() {
		var newSceneList = Array.from(Frontend.ScenesList);
		newSceneList[index] = myScene;
		Frontend.pushScenesList(newSceneList);
	}

	onMySceneChanged : {
		console.log("HELLO WORLD")
		apply()
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

			console.log("HELLO NAME IS NOW" + renameText.text)
			console.log(Frontend.ScenesList[index].name)
			console.log(myScene.name)
		}
	}

	header: ToolBar {
		visible: true

		RowLayout {
			anchors.fill: parent

			ToolButton {
				onClicked: Common.stack.pop()

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
					Common.stack.push({item:SceneEditor.createObject(), destroyOnPop:true})
				}
			}
		}
	}
}