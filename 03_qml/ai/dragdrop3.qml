import QtQuick
import QtQuick.Controls

ApplicationWindow {
    width: 400
    height: 300
    visible: true
    title: "DragHandler Example"

    Rectangle {
        anchors.fill: parent
        color: "#efefef"

        // === The drop surface ===
        Rectangle {
            id: dropSurface
            width: 200
            height: 150
            anchors.centerIn: parent
            radius: 12
            color: dropArea.containsDrag ? "#a0e3a0" : "#c0c0c0"
            border.color: "#555"

            Text {
                anchors.centerIn: parent
                text: dropArea.containsDrag ? "Drop it here!" : "Drop target"
            }

            DropArea {
                id: dropArea
                anchors.fill: parent

                onEntered: console.log("Drag entered drop surface")
                onExited: console.log("Drag exited drop surface")
                onDropped: function(drop) {
                    console.log("Item dropped:", drop.source)
                    // Change the surface color when dropped
                    dropSurface.color = "#80d080"
                }
            }
        }

        // === The draggable rectangle ===
        Rectangle {
            id: draggable
            width: 80
            height: 80
            radius: 10
            color: "#4488ff"
            border.color: "#003366"
            x: 50
            y: 50

            DragHandler {
                id: dragHandler
                target: draggable
                grabPermissions: PointerHandler.TakeOverForbidden // prevents hijacking other handlers

                onActiveChanged: {
                    if (active) {
                        // Prepare drag data
                        drag.source = draggable
                        drag.hotSpot.x = width / 2
                        drag.hotSpot.y = height / 2
                        drag.active = true
                    } else {
                        drag.active = false
                    }
                }
            }

            Drag.active: dragHandler.active
            Drag.hotSpot.x: width / 2
            Drag.hotSpot.y: height / 2
            Drag.mimeData: { "text/plain": "Rectangle" }
            Drag.supportedActions: Qt.CopyAction

            // Optional visual effect
            states: State {
                when: dragHandler.active
                PropertyChanges { target: draggable; opacity: 0.7; scale: 1.2 }
            }
            transitions: Transition {
                NumberAnimation { properties: "opacity,scale"; duration: 150 }
            }
        }
    }
}

