import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12

Page {
	property int index: -1

	header: ToolBar {
		visible: inMobileView

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
				
			Label {
				font.family: "Roboto Regular"
				font.pointSize: 14

				text: index == -1 ? "New Scene" : Frontend.ScenesList[index].name
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
				ToolTip.text: qsTr("New scene")

				onClicked: {
					Common.stack.push({item:SceneEditor.createObject(), destroyOnPop:true})
				}
			}
		}
	}
}