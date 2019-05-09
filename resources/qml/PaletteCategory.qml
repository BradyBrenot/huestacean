import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12
import Huestacean.Types 1.0
import "."
import "MaterialDesign.js" as MD

ColumnLayout {
	id: outer
	property var delegate
	property var model
	property var headerText

	property var isExpanded: true

	property alias model: repeater.model
    property alias delegate: repeater.delegate

	spacing: 0

	ItemDelegate {
		Layout.fillWidth: true
		padding: 8
		text: headerText
		font.weight: Font.Black
		highlighted: false

		LinearGradient {
			z: -2
			anchors.fill: parent
			start: Qt.point(0, 0)
			end: Qt.point(0, parent.height)

			gradient: Gradient {
				GradientStop { position: 0.0; color: "#30ffffff"}
				GradientStop { position: 1.0; color: "#00ffffff" }
			}
		}

		Label {
			font.pointSize: 18
			font.family: "Material Icons"
			text: repeater.count > 0 ? (isExpanded ? MD.icons.expand_less : MD.icons.expand_more) : ""

			anchors.verticalCenter: parent.verticalCenter
			anchors.right: parent.right
			anchors.rightMargin: 10
		}

		onClicked: { 
			isExpanded = !isExpanded;
		}
	}

	Column {
		Layout.fillWidth: true
		visible: isExpanded
		Repeater {
			id: repeater			
		}
	}
}