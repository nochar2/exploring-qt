import QtQuick
import QtQuick.Layouts
import QtQuick.Layouts
// you don't have to import "."

RowLayout {
  // Component {
  //   // property alias text: txt.text
  //   id: comp

  //   Rectangle {
  //     Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
  //     Layout.preferredWidth: 100; Layout.preferredHeight: 100
  //     color: "purple"
  //     Text {
  //       id: txt
  //       anchors.centerIn: parent
  //       font.pointSize: 40
  //       text: "sample text"
  //     }
  //   }
  // }

  // Loader {
  //   sourceComponent: comp
  //   color: "orange"
  // }
  property int w: 80
  
  RectWithText {
    color: "brown"
    label: "a"
    Layout.alignment: Qt.AlignHCenter
    Layout.preferredWidth: w; Layout.preferredHeight: w
  }

  RectWithText {
    color: "red"
    label: "b"
    Layout.alignment: Qt.AlignHCenter
    Layout.preferredWidth: w; Layout.preferredHeight: w
  }

  RectWithText {
    color: "gold"
    label: "c"
    Layout.alignment: Qt.AlignHCenter
    Layout.preferredWidth: w; Layout.preferredHeight: w
  }
  RectWithText {
    color: "limegreen"
    label: "d"
    Layout.alignment: Qt.AlignHCenter
    Layout.preferredWidth: w; Layout.preferredHeight: w
  }
}
