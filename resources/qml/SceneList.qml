import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import Huestacean.GuiHelper 1.0 as GuiHelper
import QtGraphicalEffects 1.12
import "."
import "MaterialDesign.js" as MD

Pane {
	readonly property bool inMobileView: (isMobile != undefined && isMobile) || window.width < window.height

	visible: !inMobileView
	Material.elevation: 4
	Layout.columnSpan: inMobileView ? 1 : 2
	Layout.fillHeight: true
	Layout.fillWidth: true
	Layout.minimumHeight: inMobileView ? 100 : 200

	Material.background: Material.color(Material.Grey, Material.Shade800)
	padding: inMobileView ? 0 : 30
	topPadding: 10
	bottomPadding: 20

	ColumnLayout {
		anchors.fill: parent

		RowLayout {
			Label {
				visible: !inMobileView
				font.family: "Roboto Regular"
				font.pointSize: 18

				text: "Scenes"
			}
			Item {
				Layout.fillWidth: true
			}

			RoundButton {
				font.family: "Material Icons"
				font.pointSize: 18
				text: MD.icons.add

				onClicked: {
					Common.stack.push(Common.sceneEditorComponent, {"index": -1})
				}
			}
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
					model: Frontend.ScenesList
					Button {
						id: sceneButton
						padding: 10
						Material.elevation: 6
						Material.background: "transparent"
						Layout.fillWidth: true

						background.anchors.fill: sceneButton

						Item {
							z: -2
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

							LinearGradient {
								anchors.fill: parent
								start: Qt.point(0, 0)
								end: Qt.point(0, sceneButton.height)
								source: rect

								gradient: Gradient {
									GradientStop { position: 0.0; color: "#2fffffff"}
									GradientStop { position: 0.4; color: "#00ffffff" }
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
	}
}
