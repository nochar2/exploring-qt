import QtQuick
import QtQuick.Layouts
import QtQuick.Controls


RowLayout {
    uniformCellSizes: true
    ColumnLayout {
        // uniformCellSizes: true

        Rectangle {
            color: "darkred";
            // Layout.preferredWidth: 40
            Layout.fillWidth: true
            Layout.preferredHeight: 70
            Text {
                anchors.fill: parent
                text: "Left pane"
                color: "white"
                font.pointSize: 24
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
        RowLayout {
            // uniformCellSizes: true
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "indigo"
                radius: 10
            }
        }
    }
    RowLayout {
        Rectangle {
            Layout.fillHeight: true
            Layout.fillWidth: true
            color: "violet"
        }
    }
}
// }
// Rectangle {
    // color: "midnightblue"
// }

