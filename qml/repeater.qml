import QtQuick
import QtQuick.Layouts
RowLayout {
    Repeater {
        model: ["red", "green", "blue"]

        Rectangle {
            color: modelData
            height: 20
            Layout.fillWidth: true
        }
    }
}
