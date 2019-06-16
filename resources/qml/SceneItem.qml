import QtQuick 2.12
import QtQuick.Controls 2.12

// a lot of this started off as
// https://stackoverflow.com/questions/29087710/how-to-make-a-resizable-rectangle-in-qml
// and then a lot of things happened and this is the result
// (this is no paragon of GUI code, it's pretty ugh)

Rectangle { 
	id: sceneItem

	enum SceneView {
        Top,	//x,y => x,y
        Front,  //z,y => x,y
		Side	//z,x => x,y
    }

	property int view : SceneItem.SceneView.Top
	property var scene
	property var deviceIndex: -1
	property var effectIndex: -1
	property int rulersSize: 9

	// get x, width
	// get y, height

	// set x, width
	// set y, height

	/*

	get the appropes coordinates (location, size) from my transform (x or z // y or x)
	likewise get the room's size
	get my parent's size

	scale from screen coords to "real" coords (parent size / room size)

	*/

	function getWorldLocation() {
		if(deviceIndex >= 0) {
			return myScene.devicesInScene[deviceIndex].transform.location;
		}
		else if(effectIndex > 0 && myScene.effects[effectIndex].data != undefined) {
			return myScene.effects[effectIndex].data.transform.location;
		}
	}

	function getBaseSize() {
		if(deviceIndex >= 0) {
			return myScene.devicesInScene[deviceIndex].device.size;
		}
		
		return Qt.vector3d(1,1,1)
	}

	function getWorldSize() {
		if(deviceIndex >= 0) {
			return myScene.devicesInScene[deviceIndex].transform.scale.times(getBaseSize());
		}
		else if(effectIndex > 0 && myScene.effects[effectIndex].data != undefined) {
			return myScene.effects[effectIndex].data.transform.location.scale; //* (1.0, 1.0, 1.0)
		}
	}

	function xWorldToScreen() {
		if(view == SceneItem.SceneView.Top) {
			return parent.width / myScene.size.x;
		} else {
			return parent.width / myScene.size.z;
		}
	}

	function yWorldToScreen() {
		if(view == SceneItem.SceneView.Side) {
			return parent.height / myScene.size.x;
		} else {
			return parent.height / myScene.size.y;
		}
	}

	x : {
		if(view == SceneItem.SceneView.Top) {
			return xWorldToScreen() * getWorldLocation().x;
		} else {
			return xWorldToScreen() * getWorldLocation().z;
		}
	}

	y : {
		if(view == SceneItem.SceneView.Side) {
			return yWorldToScreen() * getWorldLocation().x;
		} else {
			return yWorldToScreen() * getWorldLocation().y;
		}
	}


	width : {
		if(view == SceneItem.SceneView.Top) {
			return xWorldToScreen() * getWorldSize().x;
		} else {
			return xWorldToScreen() * getWorldSize().z;
		}
	}

	height : {
		if(view == SceneItem.SceneView.Side) {
			return yWorldToScreen() * getWorldSize().x;
		} else {
			return yWorldToScreen() * getWorldSize().y;
		}
	}

	function getName() {
		if(deviceIndex > 0) {
			return myScene.devicesInScene[deviceIndex].name;
		}
		else if(effectIndex > 0 && myScene.effects[effectIndex].data != undefined) {
			return myScene.effects[effectIndex].data + " - " + effectIndex;
		}
	}

	function canScale() {
		if(deviceIndex >= 0) {
			return myScene.devicesInScene[deviceIndex].device.canScale;
		}
		else if(effectIndex > 0 && myScene.effects[effectIndex].data != undefined) {
			return true;
		}
	}

	function canRotate() {
		if(deviceIndex >= 0) {
			return myScene.devicesInScene[deviceIndex].device.canRotate;
		}
		else if(effectIndex > 0 && myScene.effects[effectIndex].data != undefined) {
			return true;
		}
	}

	function updateSceneFromMe() {
		var loc = scene.devicesInScene[deviceIndex].transform.location;
		var scale = scene.devicesInScene[deviceIndex].transform.scale;

		//x
		if(view == SceneItem.SceneView.Top) {
			loc.x = x / xWorldToScreen();
		} else {
			loc.z = x / xWorldToScreen();
		}

		//y
		if(view == SceneItem.SceneView.Side) {
			loc.x = y / yWorldToScreen()
		} else {
			loc.y = y / yWorldToScreen()
		}

		//width
		if(view == SceneItem.SceneView.Top) {
			scale.x = width / (xWorldToScreen() * getBaseSize().x);
		} else {
			scale.z = width / (xWorldToScreen() * getBaseSize().z);
		}
		
		//height
		if(view == SceneItem.SceneView.Side) {
			scale.x = height / (yWorldToScreen() * getBaseSize().x);
		} else {
			scale.y = height / (yWorldToScreen() * getBaseSize().y);
		}

		scene.devicesInScene[deviceIndex].transform.location = loc;
		scene.devicesInScene[deviceIndex].transform.scale = scale;
	}

    border {
        width: 2
        color: "steelblue"
    }
    color: "#354682B4"

    MouseArea {     // drag mouse area
        anchors.fill: parent
        drag{
            target: parent
            minimumX: 0
            minimumY: 0
            maximumX: parent.parent.width - parent.width
            maximumY: parent.parent.height - parent.height
            smoothed: true
        }

		onMouseXChanged: {
			if(drag.active){
				updateSceneFromMe();
			}
		}

		onMouseYChanged: {
			if(drag.active){
				updateSceneFromMe();
			}
		}
    }

    Rectangle {
		visible: canScale()
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        color: "steelblue"
        anchors.horizontalCenter: parent.left
        anchors.verticalCenter: parent.verticalCenter

        MouseArea {
            anchors.fill: parent
            drag{ target: parent; axis: Drag.XAxis }
            onMouseXChanged: {
                if(drag.active){
                    sceneItem.width = sceneItem.width - mouseX
                    sceneItem.x = sceneItem.x + mouseX
                    if(sceneItem.width < 30)
                        sceneItem.width = 30
                }

				updateSceneFromMe();
            }
        }
    }

    Rectangle {
		visible: canScale()
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        color: "steelblue"
        anchors.horizontalCenter: parent.right
        anchors.verticalCenter: parent.verticalCenter

        MouseArea {
            anchors.fill: parent
            drag{ target: parent; axis: Drag.XAxis }
            onMouseXChanged: {
                if(drag.active){
                    sceneItem.width = sceneItem.width + mouseX
                    if(sceneItem.width < 50)
                        sceneItem.width = 50
                }

				updateSceneFromMe();
            }
        }
    }

    Rectangle {
		visible: canScale()
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        x: parent.x / 2
        y: 0
        color: "steelblue"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.top

        MouseArea {
            anchors.fill: parent
            drag{ target: parent; axis: Drag.YAxis }
            onMouseYChanged: {
                if(drag.active){
                    sceneItem.height = sceneItem.height - mouseY
                    sceneItem.y = sceneItem.y + mouseY
                    if(sceneItem.height < 50)
                        sceneItem.height = 50
                }

				updateSceneFromMe();
            }
        }
    }


    Rectangle {
		visible: canScale()
        width: rulersSize
        height: rulersSize
        radius: rulersSize
        x: parent.x / 2
        y: parent.y
        color: "steelblue"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.bottom

        MouseArea {
            anchors.fill: parent
            drag{ target: parent; axis: Drag.YAxis }
            onMouseYChanged: {
                if(drag.active){
                    sceneItem.height = sceneItem.height + mouseY
                    if(sceneItem.height < 50)
                        sceneItem.height = 50
                }

				updateSceneFromMe();
            }
        }
    }
}