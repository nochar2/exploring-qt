import QtQuick
import QtQuick.Controls.Material
// import QtQuick.Controls.Universal
import QtQuick.Effects
import QtQuick.Layouts

ApplicationWindow {
    id: win
    Material.theme: Material.System
    visible: true


    RowLayout {
        anchors.centerIn: parent

        // let's not care about droparea for now,
        // I don't know how to work with it...

        Frame {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            // anchors.fill: parent

            RowLayout {
                Label { text: "first" }
                Frame {
                    RowLayout {
                        Label { text: "second" }
                        Frame {
                            Label { text: "end" }
                        }

                    }
                }
            }
        
            // radius: 5
            // width:  da.containsDrag ? 80 : 60
            // height: da.containsDrag ? 55 : 40
            width: 80
            height: 55

            // color: "green"
            // border.color: "purple"
            // border.width: 5

            visible: true
        }
    }


    // -- thing that I can drag around
    RectangularShadow {
        anchors.fill: r1
        blur: 30
        color: Qt.darker(r1.color, 1.6)
        // offset: { x: 100 y: 100 } // doesn't seem to work
    }
    Rectangle {
        id: r1
        // x: 30; y: 30
        width: 30; height: 30
        color: "deepskyblue"
        radius: 10

        Drag.active: dh.active

        DragHandler {
            id: dh
            // xAxis.enabled: false
            xAxis.minimum: 0
            yAxis.minimum: 0
            yAxis.maximum: win.height - parent.height
            xAxis.maximum: win.width  - parent.width
            
            // -- idk what is this for
            // snapMode: DragHandler.SnapAlways
            
            onActiveChanged:
                if (this.active) {
                    console.log("drag started");
                } else {
                    console.log("drag finished");
                }
        }
        // -- using this, you get ~1 frame delay and then
        // -- the cursor position is completely detached from
        // -- the square. DragHander doesn't have this issue.
        // MouseArea {
        //     anchors.fill: parent
        //     id: dragArea
        //     drag.target: parent
        // }
    }
}

