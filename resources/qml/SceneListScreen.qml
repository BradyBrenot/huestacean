import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12
import "."
import "MaterialDesign.js" as MD

Page {
	readonly property bool inMobileView: (isMobile != undefined && isMobile) || window.width < window.height

	signal clickedBack
	

	id: homeView
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

				text: "Scenes"
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
	}
}