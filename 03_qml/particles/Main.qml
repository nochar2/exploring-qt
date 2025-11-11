import QtQuick
import QtQuick.Layouts

Item {
    width: 800
    height: 600

    RowLayout
    {
        anchors.fill: parent
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
}
