import QtQuick
import QtQuick.Layouts
Item {
    width: 800
    height: 600

    RowLayout {
        anchors.fill: parent
        Repeater {
            model: ["red", "green", "blue"]

            Rectangle {
                color: modelData
                height: 20
                Layout.fillWidth: true
            }
        }
    }
}
