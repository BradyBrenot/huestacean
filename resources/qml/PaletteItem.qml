import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12
import Huestacean.Types 1.0
import "."
import "MaterialDesign.js" as MD

ItemDelegate {
	id: outerButton

	font.pointSize: 12
	Material.elevation: 6
	Material.background: "transparent"
	Layout.fillWidth: true
	padding: 10

	property var isOddNumbered: false
	property var isHighlighted: false
	property var innerButtonText: "NONE"
	property var innerButtonTooltip: "NONE"

	signal innerButtonClicked()
	signal clicked()

	Connections {
        target: innerButton
        onClicked: {
            outerButton.innerButtonClicked()
        }
    }

	MouseArea {
		anchors.fill: parent
		onClicked: outButton.clicked()
	}

	Item {
		visible: isHighlighted
		z: -2
		anchors.fill: parent
		Rectangle {
			anchors.fill:parent
			color: "white"
			opacity: 0.1
		}
	}

	Item {
		visible: !isOddNumbered
		z: -3
		anchors.fill: parent
		Rectangle {
			anchors.fill:parent
			color: "black"
			opacity: 0.1
		}
	}

							
	Button {
		id: innerButton
		height: 40

		//font.family: "Material Icons"
		font.weight: Font.Light
		text: innerButtonText

		ToolTip.visible: hovered || pressed
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
		ToolTip.text: innerButtonTooltip

		anchors.verticalCenter: parent.verticalCenter
		anchors.right: parent.right
		anchors.rightMargin: 10
	}
}