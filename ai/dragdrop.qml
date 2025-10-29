import QtQuick //2.15
import QtQuick.Controls // 2.15

ApplicationWindow {
    visible: true
    width: 800
    height: 400
    title: "Drag and Drop Container Demo"

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#f0f0f0"
    }

    // ===== Draggable rectangles outside the container =====
    Repeater {
        model: 3
        Rectangle {
            width: 80; height: 80; radius: 10
            color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1.0)
            x: 100 + index * 100
            y: 50

            Drag.active: dragArea.drag.active
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.source: parent
            Drag.mimeData: { "application/x-item": index }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                drag.target: parent
            }
        }
    }

    // ===== Container that accepts drops and resizes =====
    Rectangle {
        id: container
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 60
        height: 120
        width: row.childrenRect.width + 20
        radius: 10
        color: "#ffffff"
        border.color: "steelblue"
        border.width: 2

        Row {
            id: row
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            spacing: 10
        }

        // Accept dropped items
        DropArea {
            anchors.fill: parent
            keys: ["application/x-item"]

            onDropped: (drop) => {
                let rect = container.createObject(Rectangle {
                    width: 80; height: 80; radius: 10;
                    color: Qt.rgba(Math.random(), Math.random(), Math.random(), 1.0)
                }
                row.addItem(rect)
            }
        }

        // Subtle animation when resizing
        Behavior on width { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: container.top
        anchors.bottomMargin: 20
        text: "Drag rectangles into the container"
        font.pixelSize: 20
        color: "#444"
    }
}

