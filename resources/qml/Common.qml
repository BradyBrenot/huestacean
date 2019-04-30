pragma Singleton
import QtQuick 2.12
import QtRemoteObjects 5.12
import QtQuick.Controls 2.12

QtObject {
    property StackView stack
	property Component sceneEditorComponent: Qt.createComponent("SceneEditor.qml", Component.PreferSynchronous);
}