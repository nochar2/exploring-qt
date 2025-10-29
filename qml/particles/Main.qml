import QtQuick
import QtQuick.Layouts


RowLayout
// Row
{
    Repeater {
        id: rep
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        model: ["deepskyblue", "orange", "limegreen", "violet", "pink"]
        // model: ["deepskyblue"]
        // ColoredRectangle {
        //     cl: rep.modelData
        // }
        // ColoredRectangle {
        //     implicitWidth: 100; implicitHeight: 100
        //     color: modelData
        // }

        GlowingPot {
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            color: modelData
            Layout.fillWidth: true
        }

    }
}
