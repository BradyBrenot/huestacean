import QtQml 2.2
import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import Huestacean 1.0
import "qrc:/qml"

Pane {
    id: home

	contentWidth: mainColumn.implicitWidth
    contentHeight: mainColumn.implicitHeight

    ColumnLayout {
		id: mainColumn
		spacing: 10

		Bridges {
			visible: !hasAtLeastOneConnection
		}

		ColumnLayout {
			spacing: 10

			RowLayout {
				spacing: 20

				ColumnLayout {
					Layout.fillWidth: true

					Label {
						text: "Monitor"
					}

					RowLayout {
						Button {
							id:redetectButton
							focus: true
							text: "Redetect"
							onClicked: Huestacean.detectMonitors()

							KeyNavigation.right: monitorComboBox
							KeyNavigation.down: entimagepreview
						}

						ComboBox {
							id:monitorComboBox
							Layout.fillWidth: true
							Layout.maximumWidth: 200

							currentIndex: 0
							model: Huestacean.monitorsModel
							textRole: "asString"
							onCurrentIndexChanged: Huestacean.setActiveMonitor(currentIndex)

							KeyNavigation.right: egroupRedetect
							KeyNavigation.down: entimagepreview
						}
					}

					//Rectangle { height: 200; width: 300; }

					Image {
						id: entimage
						source: "image://entimage/ent"
							
						Layout.fillWidth: true
                        Layout.maximumWidth: Qt.platform.os == "android" ? 200 : 400
                        Layout.maximumHeight: Qt.platform.os == "android" ? 110 : 225

                        Layout.preferredWidth: Qt.platform.os == "android" ? 200 : 400
                        Layout.preferredHeight: Qt.platform.os == "android" ? 110 : 225

						cache: false
						asynchronous: true
						smooth: false

						Timer {
							interval: entimagepreview.checked ? 20 : 200; 
                            running: Huestacean.syncing && mainWindow.visibility != Window.Minimized && mainWindow.visible != Window.Hidden;
							repeat: true;
							onTriggered: {
                                entimage.source = entimage.source == "image://entimage/ent" ? "image://entimage/ent1" : "image://entimage/ent"
							}
						}
					}
				}

				ColumnLayout {
					Layout.fillWidth: true

					Label {
						text: "Entertainment group"
					}

					RowLayout {
						Layout.maximumWidth: 300

						Button {
							id:egroupRedetect
							text: "Redetect"
							onClicked: Huestacean.refreshGroups()

							KeyNavigation.right: entertainmentComboBox
							KeyNavigation.down: entimagepreview
						}

						ComboBox {
							id: entertainmentComboBox
							Layout.fillWidth: true
							currentIndex: 0
							model: Huestacean.entertainmentGroupsModel
							textRole: "asString"

							KeyNavigation.right: entertainmentComboBox
							KeyNavigation.down: entimagepreview

							property var lights: undefined

							onModelChanged: {
								updateLights();
							}

							onCurrentIndexChanged: {
								updateLights()
							}

							function updateLights() {
								if(model) {
									if(lights) {
										for (var i in lights) {
											lights[i].destroy();
										}
									}

									lights = [];
									for(var i = 0; i < model[currentIndex].numLights(); i++) {
										var light = model[currentIndex].getLight(i);
										var l = lightComponent.createObject(groupImage);
										l.name = light.id;
										l.index = i
										l.setPos(light.x, light.z)
										lights.push(l);
									}
								}
							}
						}
					}

					Rectangle {
						color: "black"
                        height: Qt.platform.os == "android" ? 110 : 220;
                        width: Qt.platform.os == "android" ? 110 : 220;
						border.width: 1
						border.color: "#414141"

						Image { 
							anchors.centerIn: parent
							id: groupImage
                            height: Qt.platform.os == "android" ? 100 : 200;
                            width: Qt.platform.os == "android" ? 100 : 200;
							source: "qrc:/images/egroup-xy.png"
						}
					}						
				}
			}

			RowLayout {
				spacing: 20

				CheckBox {
					id:entimagepreview
					text: "Fast visualizer"
					checked: false

					KeyNavigation.down: startSyncButton

				}

				Label {
					text: "Frame read:" + Huestacean.frameReadElapsed + "ms"
				}				

				Label {
					visible: false
					text: "Net send:" + Huestacean.messageSendElapsed + "ms"
				}
			}

			RowLayout {
				spacing: 20

				Button {
					id:startSyncButton
					text: Huestacean.syncing ? "Stop sync" : "Start sync"
					onClicked: Huestacean.syncing ? Huestacean.stopScreenSync() : Huestacean.startScreenSync(entertainmentComboBox.model[entertainmentComboBox.currentIndex])

					KeyNavigation.right: frameslider
					KeyNavigation.down: minLumaSlider
				}

				Column {
					visible: (Qt.platform.os != "android" && Qt.platform.os != "osx")

					Label {
						text: "Capture interval"
					}

					Slider {
						id: frameslider

						Component.onCompleted: value = Huestacean.captureInterval / 100
						onValueChanged: {
							Huestacean.captureInterval = value * 100
						}

						KeyNavigation.right: skipslider
						KeyNavigation.down: minLumaSlider
					}

					Label {
						text: Huestacean.captureInterval + " milliseconds"
					}
				}

				Column {
					visible: !Huestacean.mipMapGenerationEnabled

					Label {
						text: "Skip pixels"
					}

					Slider {
						id: skipslider
						Component.onCompleted: value = Huestacean.skip / 128
						onValueChanged: {
							Huestacean.skip = value * 128
						}

						KeyNavigation.down: minLumaSlider
					}

					Label {
						text: Huestacean.mipMapGenerationEnabled ? "Not needed on this platform" : Huestacean.skip
					}
				}
			}

			RowLayout {
				spacing: 20
				
				Column {
					Label {
						text: "Min brightness"
					}

					Slider {
						id:minLumaSlider

						Component.onCompleted: value = Huestacean.minLuminance
						onValueChanged: {
							Huestacean.minLuminance = value
						}

						KeyNavigation.right: maxLumaSlider
						KeyNavigation.down: chromaBoostSlider
					}

					Label {
						text: Huestacean.minLuminance.toFixed(2)
					}
				}

				Column {
					Label {
						text: "Max brightness"
					}

					Slider {
						id:maxLumaSlider

						Component.onCompleted: value = Huestacean.maxLuminance
						onValueChanged: {
							Huestacean.maxLuminance = value
						}
					}

					Label {
						text: Huestacean.maxLuminance.toFixed(2)
					}
				}
			}

			RowLayout {
				spacing: 20

				Column {
					Label {
						text: "Saturation boost"
					}

					Slider {
						id:chromaBoostSlider

						Component.onCompleted: value = Huestacean.chromaBoost / 3
						onValueChanged: {
							Huestacean.chromaBoost = value * 3
						}

						KeyNavigation.right: lumaBoostSlider
						KeyNavigation.down: centerSlownessSlider
					}

					Label {
						text: Huestacean.chromaBoost.toFixed(2)
					}
				}

				Column {
					Label {
						text: "Brightness boost"
					}

					Slider {
						id: lumaBoostSlider

						Component.onCompleted: value = Huestacean.lumaBoost / 5
						onValueChanged: {
							Huestacean.lumaBoost = value * 5
						}

						KeyNavigation.down: centerSlownessSlider
					}

					Label {
						text: Huestacean.lumaBoost.toFixed(2)
					}
				}
			}

			RowLayout {
				spacing: 20

				Column {
					Label {
						text: "Center slowness"
					}

					Slider {
						id: centerSlownessSlider

						Component.onCompleted: {
							var slowness = Huestacean.centerSlowness
							from = 1.0
							to = 20.0
							value = slowness
						}
						onValueChanged: {
							Huestacean.centerSlowness = value
						}

						KeyNavigation.right: sideSlownessSlider
						KeyNavigation.down: resetButton
					}

					Label {
						text: Huestacean.centerSlowness.toFixed(2)
					}
				}

				Column {
					Label {
						text: "Side slowness"
					}

					Slider {
						id: sideSlownessSlider

						Component.onCompleted: {
							var slowness = Huestacean.sideSlowness
							from = 1.0
							to = 20.0
							value = slowness
						}
						onValueChanged: {
							Huestacean.sideSlowness = value
						}

						KeyNavigation.down: resetButton
					}

					Label {
						text: Huestacean.sideSlowness.toFixed(2)
					}
				}
			}

			Column {
				Button {
					id:resetButton
					text: "Reset settings"
					onClicked: {
						Huestacean.ResetSettings()

						centerSlownessSlider.value = Huestacean.centerSlowness
						sideSlownessSlider.value = Huestacean.sideSlowness
						lumaBoostSlider.value = Huestacean.lumaBoost / 5
						chromaBoostSlider.value = Huestacean.chromaBoost / 3
						minLumaSlider.value = Huestacean.minLuminance
						maxLumaSlider.value = Huestacean.maxLuminance
						skipslider.value = Huestacean.skip / 128
						frameslider.value = Huestacean.captureInterval / 100
					}
				}
			}

		}
	}

	Component {
        id: lightComponent

		Rectangle { 
			id: lightIcon
			color: "blue";
			height: 20; 
			width: 20; 
			radius: 5

			property var name: "INVALID"
			property var index: 0

			function setPos(inX, inY) {
				x = ((1 + inX) / 2.0) * groupImage.width - width/2
				y = ((1 - inY) / 2.0) * groupImage.height - height/2
			}

			function updatePosition(inSave) {
				var bridgeX = 2 * (lightIcon.x + lightIcon.width/2) / groupImage.width - 1
				var bridgeZ = 1 - 2 * (lightIcon.y + lightIcon.height/2) / groupImage.height

				entertainmentComboBox.model[entertainmentComboBox.currentIndex].updateLightXZ(index, bridgeX, bridgeZ, true);
			}

			onXChanged : updatePosition(false);
			onYChanged : updatePosition(false);

			Label {
				anchors.centerIn: parent
				text: lightIcon.name
			}

			MouseArea {
				id: mouseArea
				anchors.fill: parent
				drag.target: lightIcon
				drag.axis: Drag.XAndYAxis

				drag.minimumX: 0 - width/2
				drag.maximumX: groupImage.width - width/2

				drag.minimumY: 0 - height/2
				drag.maximumY: groupImage.height - height/2

				drag.onActiveChanged: {
					if(!drag.active) {
						var bridgeX = 2 * (lightIcon.x + lightIcon.width/2) / groupImage.width - 1
						var bridgeZ = 1 - 2 * (lightIcon.y + lightIcon.height/2) / groupImage.height

						updatePosition(true);
					}
				}
			}
		}
	}
}
