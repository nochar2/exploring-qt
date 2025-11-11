import QtQuick
import QtQml
import DraggableItem

Window {
    width: 800
    height: 600
    visible: true
    title: "Draggable Container"

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"
    }

    // Container for items
    Rectangle {
        id: container
        x: 100
        y: 100
        width: Math.max(20, contentArea.childrenRect.width + 20)
        height: Math.max(20, contentArea.childrenRect.height + 20)
        border.color: "#333"
        border.width: 2
        color: "transparent"

        // This rectangle helps track bounds of contained items
        Rectangle {
            id: contentArea
            x: 10
            y: 10
            width: 0
            height: 0
            color: "transparent"
            
            onChildrenRectChanged: container.updateSize()
        }

        function updateSize() {
            width = Math.max(20, contentArea.childrenRect.width + 20)
            height = Math.max(20, contentArea.childrenRect.height + 20)
        }
    }

    // Draggable rectangle 1
    DraggableItem  {
        id: item1
        x: 50
        y: 150
        width: 80
        height: 80
        color: "#ff6b6b"
        label: "1"
        container: container
    }

    // Draggable rectangle 2
    DraggableItem {
        id: item2
        x: 250
        y: 200
        width: 80
        height: 80
        color: "#4ecdc4"
        label: "2"
        container: container
    }

    // Draggable rectangle 3
    DraggableItem  {
        id: item3
        x: 450
        y: 150
        width: 80
        height: 80
        color: "#95e1d3"
        label: "3"
        container: container
    }
}

