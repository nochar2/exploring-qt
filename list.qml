import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    width: 600
    height: 480
    visible: true

    ListModel {
        id: colorModel
        ListElement { color: "red" }
        ListElement { color: "green" }
        ListElement { color: "blue" }
    }

    Row {
        // anchors.horizontalCenter: parent.horizontalCenter
        // anchors.verticalCenter: parent.verticalCenter
        anchors.centerIn: parent
        Repeater {
            model: colorModel

            delegate: Rectangle {
                width: 70
                height: 70
                color: model.color
                radius: 8
            }

        
        }
    }

    
}
