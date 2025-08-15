import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

RowLayout {
    anchors.fill: parent
TableView {
    Layout.fillWidth: true
    Layout.fillHeight: true
    columnSpacing: 1
    rowSpacing: 1
    clip: true

    model: TableModel {
        TableModelColumn { display: "name" }
        TableModelColumn { display: "color" }

        rows: [
            {
                "name": "cat",
                "color": "black"
            },
            {
                "name": "dog",
                "color": "brown"
            },
            {
                "name": "bird",
                "color": "white"
            }
        ]
    }

    delegate: Rectangle {
        implicitWidth: 100
        implicitHeight: 50
        border.width: 1

        Text {
            text: display
            anchors.centerIn: parent
        }
    }
}
}
