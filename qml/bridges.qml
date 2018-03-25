import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Huestacean 1.0

Pane {
    id: bridges
	 
    ColumnLayout
	{
		id: mainColumn
		anchors.fill: parent

		spacing: 10

		Row {
			spacing: 10

			Button {
				id: searchButton
				focus: true
				text: qsTr("Search")
				onClicked: {
					searchIndicator.searching = true
					searchTimer.start()
					Huestacean.bridgeDiscovery.startSearch()
				}

				Keys.onPressed: {
					console.log("event.key" + event.key);
				}

				KeyNavigation.down: bridgesGrid
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

			FocusScope {
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
									text: modelData.friendlyName
								}

								Label {
									text: modelData.address
								}
						
								Label {
									text: modelData.id
								}

								Label {
									text: "Connected: " + modelData.connected
								}
							}

							Button {
								focus: visible
								anchors.top: parent.top
								anchors.bottom: parent.bottom
								Layout.column: 1
								text: "Connect"
								visible: !modelData.connected && !modelData.wantsLinkButton

								onClicked: modelData.connectToBridge()
							}

							Button {
								focus: visible
								anchors.top: parent.top
								anchors.bottom: parent.bottom
								Layout.column: 1
								text: "Link"
								visible: !modelData.connected && modelData.wantsLinkButton

								onClicked: {
									linkPopup.bridge = modelData
									linkPopup.open()
								}
							}

							Button {
								anchors.top: parent.top
								anchors.bottom: parent.bottom
								Layout.column: 1
								text: "Forget"
								visible: modelData.connected

								onClicked: modelData.resetConnection()
							}
						}
					}
				}
			}
		}

		GridView {
			id: bridgesGrid
			clip: true
			Layout.fillWidth: true
			Layout.minimumHeight: 200

			cellWidth: 200; cellHeight: 100
			model: Huestacean.bridgeDiscovery.model
			delegate: bridgeDelegate

			KeyNavigation.down: manualIP
		}

		Row {
			spacing: 10
			TextField {
				id: manualIP
				focus: true
				width: 150
				placeholderText: "0.0.0.0"

				KeyNavigation.right: manuallyAdd
			}
			Button {
				id: manuallyAdd
				text: "Manually add IP"
				onClicked: Huestacean.bridgeDiscovery.manuallyAddIp(manualIP.text)
				KeyNavigation.up: manualIP
			}
		}
	}

	Popup {
		id: linkPopup
		x: (mainColumn.Window.width - width) / 2
		y: (mainColumn.Window.height - height) / 2

		modal: true
		focus: true
		closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

		property var bridge

		Timer {
			id: linkKillTimer
			interval: 10000;
			repeat: false
			onTriggered: {
				linkPopup.close()
			}
		}

		Timer {
			id: linkTimer
			interval: 2000;
			repeat: true
			onTriggered: {
				linkPopup.bridge.connectToBridge();
			}
		}

		onOpened: {
			linkTimer.start()
			linkKillTimer.start()
		}
		onClosed: {
			linkTimer.stop()
			linkKillTimer.stop()
		}

		Column {
			anchors.fill: parent
			spacing: 10

			Label { 
				font.bold: true
				text: "Linking with " + (linkPopup.bridge ? linkPopup.bridge.address : "INVALID")
			}

			Connections { 
				target: linkPopup.bridge

				onConnectedChanged: {
					if(linkPopup.bridge.connected) {
						linkPopup.close()
					}
				}
			}

			Label { text: "Press the Link button on the bridge"}
			Row {
				spacing: 10
				anchors.horizontalCenter: parent.horizontalCenter
			}
		}
	}

	Popup {
		id: entertainmentGroupsWarningPop
		x: (mainColumn.width - width) / 2
		y: (mainColumn.Window.height - height) / 2
		width: 300

		modal: true
		focus: true
		closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

		property var bridge

		Column {
			spacing: 10

			Label {
				width: 280
				text: '<html>
					  You don\'t seem to have any entertainment groups.
					  At the moment, Huestacean cannot do this for you.
					  You need to create an entertainment group in the Hue Android or iOS app.
					  Philips has a video demonstrating this, <a href="https://www.youtube.com/watch?v=_N7VNJM_8js">here on their Youtube channel</a>
					  </html>'

				wrapMode: Label.Wrap

				onLinkActivated: Qt.openUrlExternally(link)
			}

			Row {
				spacing: 10
				anchors.horizontalCenter: parent.horizontalCenter
				Button {
					text: "Redetect"
					onClicked: Huestacean.refreshGroups()
				}

				Button {
					text: "Close"
					onClicked: entertainmentGroupsWarningPop.close()
				}
			}
		}
	}

	Connections { 
		target: Huestacean

		//onEntertainmentGroupsChanged: {
		//	if(Huestacean.entertainmentGroupsModel.length == 0) {
		//		entertainmentGroupsWarningPop.open()
		//	} else {
		//		entertainmentGroupsWarningPop.close()
		//	}
		//}
	}
}
