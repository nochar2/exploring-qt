import QtQuick

Rectangle {
    id: item
    width: 80
    height: 80
    radius: 5
    required property string label
    required property Rectangle container

    Text {
        anchors.centerIn: parent
        text: parent.label
        font.pixelSize: 24
        font.bold: true
        color: "white"
    }

    MouseArea {
        anchors.fill: parent
        drag.target: parent
        drag.smoothed: true

        onReleased: {
            // Check if item is within container bounds
            const containerRect = mapToItem(null, 0, 0, item.width, item.height)
            const containerGlobalRect = {
                x: container.mapToItem(null, 0, 0).x,
                y: container.mapToItem(null, 0, 0).y,
                width: container.width,
                height: container.height
            }

            const isInside = containerRect.x >= containerGlobalRect.x &&
                           containerRect.y >= containerGlobalRect.y &&
                           (containerRect.x + item.width) <= (containerGlobalRect.x + containerGlobalRect.width) &&
                           (containerRect.y + item.height) <= (containerGlobalRect.y + containerGlobalRect.height)

            if (isInside && item.parent !== container) {
                // Move item into container
                const localPos = container.mapFromItem(null, containerRect.x, containerRect.y)
                item.parent = container.contentArea
                item.x = localPos.x - 10
                item.y = localPos.y - 10
                container.updateSize()
            } else if (!isInside && item.parent !== item.parent.parent) {
                // Move item out of container
                const globalPos = item.mapToItem(null, 0, 0)
                item.parent = item.parent.parent.parent
                item.x = globalPos.x
                item.y = globalPos.y
                container.updateSize()
            }
        }
    }
}
