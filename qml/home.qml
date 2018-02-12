import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Huestacean 1.0

Pane {
    id: home

    Column {
        anchors.fill: parent

		spacing: 10

		GroupBox {
			id: bridgesBox
			anchors.left: parent.left
			anchors.right: parent.right
			title: "Bridges"

			Column
			{
				anchors.left: parent.left
				anchors.right: parent.right
				spacing: 10

				Row {
					spacing: 10

					Button {
						text: qsTr("Search")
						onClicked: {
							searchIndicator.searching = true
							searchTimer.start()
							Huestacean.bridgeDiscovery.startSearch()
						}
					}

					BusyIndicator {
						id: searchIndicator
						property bool searching
						visible: searching
					}

					Timer {
						id: searchTimer
						interval: 5000;
						repeat: false
						onTriggered: searchIndicator.searching = false
					}
				}

				Component {
					id: bridgeDelegate

					Item {
						width: bridgesGrid.cellWidth
						height: bridgesGrid.cellHeight

						Rectangle {
							color: "#0FFFFFFF"
							radius: 10
							anchors.fill: parent
							anchors.margins: 5

							GridLayout {
								columns: 2
								anchors.fill: parent
								anchors.margins: 5

								Column {
									Layout.column: 0

									Label {
										font.bold: true
										text: model.item.friendlyName
									}

									Label {
										text: model.item.address
									}
						
									Label {
										text: model.item.id
									}

									Label {
										text: "Connected: " + model.item.connected
									}
								}

								Button {
									anchors.top: parent.top
									anchors.bottom: parent.bottom
									Layout.column: 1
									text: "Connect"
									visible: !model.item.connected && !model.item.wantsLinkButton

									onClicked: model.item.connectToBridge()
								}

								Button {
									anchors.top: parent.top
									anchors.bottom: parent.bottom
									Layout.column: 1
									text: "Link"
									visible: !model.item.connected && model.item.wantsLinkButton

									onClicked: {
										linkPopup.bridge = model.item
										linkPopup.open()
									}
								}

								Button {
									anchors.top: parent.top
									anchors.bottom: parent.bottom
									Layout.column: 1
									text: "Forget"
									visible: model.item.connected

									onClicked: model.item.resetConnection()
								}
							}
						}
					}
				}

				GridView {
					id: bridgesGrid
					clip: true
					anchors.left: parent.left
					anchors.right: parent.right
					height: 125

					cellWidth: 200; cellHeight: 100
					model: Huestacean.bridgeDiscovery.model
					delegate: bridgeDelegate

					ScrollBar.vertical: ScrollBar {}
				}

				Row {
					spacing: 10
					TextField {
						focus: true
						width: 150
						placeholderText: "0.0.0.0"
					}
					Button {
						text: "(TODO) Manually add IP"
					}
				}
			}
		}
		
		GroupBox {
			anchors.left: parent.left
			anchors.right: parent.right

			title: "Screen sync settings"

			Column {
				anchors.left: parent.left
				anchors.right: parent.right
				spacing: 10

				Label {
					text: "Monitor"
				}

				Row {
					ComboBox {
						currentIndex: 0
						width: 400
						model: Huestacean.monitorsModel
						textRole: "asString"
						onCurrentIndexChanged: Huestacean.setActiveMonitor(currentIndex)
					}

					Button {
						text: "Redetect"
						onClicked: Huestacean.detectMonitors()
					}
				}
				
				RowLayout {
					anchors.left: parent.left
					anchors.right: parent.right

					Rectangle { height: 200; width: 300; }
				}
			}
		}
    }

	Popup {
		id: linkPopup
		x: (parent.width - width) / 2
		y: (parent.height - height) / 2

		modal: true
		focus: true
		closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

		property var bridge

		Column {
			anchors.fill: parent
			spacing: 10

			Label { 
				font.bold: true
				text: "Linking with " + (linkPopup.bridge ? linkPopup.bridge.address : "INVALID")
			}

			Label { text: "Press the Link button on the bridge, then click \"Link now\""}
			Row {
				spacing: 10
				anchors.horizontalCenter: parent.horizontalCenter
				Button {
					text: "Link now"
					onClicked: {
						if(linkPopup.bridge)
							linkPopup.bridge.connectToBridge();

						linkPopup.close();
					}
				}
				Button {
					text: "Cancel"
					onClicked: linkPopup.close()
				}
			}
		}
	}
}
