import QtQuick
import QtQuick.Layouts

Item {

    RowLayout {
        anchors.centerIn: parent

        DropArea {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            implicitWidth:  containsDrag ? 80 : 60
            implicitHeight: containsDrag ? 55 : 40

            Rectangle {
                anchors.fill: parent
                radius: 5

                // color: "green"
                border.color: "purple"
                border.width: 5

                visible: true
                // width: parent.containsDrag ? 100 : 50;
            }
        }
    }

    Rectangle {
        id: r1
        x: 30; y: 30
        width: 30; height: 30
        color: "deepskyblue"

        // -- I don't know how to use this...
        // onDragStarted: { console.log("drag started!"); }
        Drag.active: dragArea.drag.active
        Drag.hotSpot.x: this.width/2
        Drag.hotSpot.y: this.height/2

        MouseArea {
            anchors.fill: parent
            id: dragArea

            drag.target: parent
        }
    }
}
