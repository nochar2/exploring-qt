import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal


ApplicationWindow {
    visible: true
    Universal.theme: Universal.System

    Rectangle {
        id: root

        width: 300
        height: 400
        color: Universal.background

        Component {
            id: dragDelegate

            Rectangle {
                id: content
                color: Universal.background

                required property string name
                required property string type
                required property string size
                required property int age

                width: view.width
                height: column.implicitHeight + 4

                border.width: 1
                border.color: "lightsteelblue"

                radius: 2

                Column {
                    id: column
                    anchors {
                        fill: parent
                        margins: 2
                    }

                    Text { text: qsTr('Name: ') + content.name }
                    Text { text: qsTr('Type: ') + content.type }
                    Text { text: qsTr('Age: ') + content.age }
                    Text { text: qsTr('Size: ') + content.size }
                }
            }
        }
        ListView {
            id: view

            anchors {
                fill: parent
                margins: 2
            }

            model: PetsModel {}
            delegate: dragDelegate

            spacing: 4
            cacheBuffer: 50
        }
    }
}
