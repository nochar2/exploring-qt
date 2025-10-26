import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ApplicationWindow {
  width: 640
  height: 480
  visible: true
  RowLayout {
    anchors.fill: parent
    Rectangle {
      Layout.fillHeight: true
      Layout.preferredWidth: 200
      color: "red"
    }
    Rectangle {
      Layout.fillHeight: true
      Layout.preferredWidth: 200
      color: "orange"
    }
    Rectangle {
      Layout.fillHeight: true
      Layout.fillWidth: true
      color: "green"
    }
  }

}
