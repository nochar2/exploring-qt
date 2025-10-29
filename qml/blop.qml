import QtQuick //2.0
import QtQuick.Layouts //2.0

// RowLayout {
//     id: rowLayout
//     anchors.fill: parent

RowLayout {

    Rectangle {
        id: rectangle
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.preferredWidth: 100
        Layout.preferredHeight: 100

        color: "white"
        border.color: "purple"
        border.width: 8
        radius: 10

        MouseArea {
            anchors.fill: parent
            onClicked: {
                blop.stop()
                blop.start()
            }
        }

        ParallelAnimation {
            id: blop
            NumberAnimation  {
                target: rectangle; property: "Layout.preferredWidth"
                from: 0
                to: 200
                duration: 500
                easing { type: Easing.OutQuad; }
            }
            NumberAnimation {
                target: rectangle; property: "opacity"
                from: 1
                to: 0
                duration: 500
                easing { type: Easing.OutQuad; }
            }
        }
    }

}
