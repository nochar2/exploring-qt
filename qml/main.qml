import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Universal
// import Qt.labs.qmlmodels

// NOTES:
// - if i put a tableview to a columnlayout, it disappears. Why?
//   - you have to set some width or something, I forgot

ApplicationWindow {
  Universal.theme: Universal.System
  Universal.accent: Universal.Indigo

  width: 640
  height: 480
  visible: true
  title: "Hello World"


  ColumnLayout {
    anchors.fill: parent

    MenuBar {
      Menu {
        title: "File"
        Action { text: "New" }
        Action { text: "Open" }
        Action { text: "Save" }
        MenuSeparator {}
        Action { text: "Exit" }
      }
      // Menu {
      //   title: "Edit"
      //   Action {text: "Setup custom snaps..."}
      // }
      Menu {
        title: "Help"
        Action {text: "Guide"}
        Action {text: "About"}
      }
    }

    TabBar {
      wheelEnabled: true
      TabButton { text: "File 1" }
      TabButton { text: "File 2" }
    }

    RowLayout {
      id: main
      Layout.bottomMargin: 10; Layout.leftMargin: 10; Layout.rightMargin: 10

      ColumnLayout {
        id: sidebar_toggles
        Layout.preferredWidth: 25
        Layout.maximumWidth: 25
        Layout.alignment: Qt.AlignTop
        Button {
          contentItem: Text {
            text: "Tree"
            wrapMode: Text.WrapAnywhere
          }
          Layout.fillWidth: true
          Layout.preferredHeight: 100
          onClicked: function() { sidebar.visible = !sidebar.visible; }
        }
        Button {
          contentItem: Text {
            text: "Snaps"
            wrapMode: Text.WrapAnywhere
          }
          Layout.fillWidth: true
          Layout.preferredHeight: 100
        }
      }

      ColumnLayout {
        id: sidebar

        ListView {
          Layout.fillHeight: true
          Layout.preferredWidth: 250
          model: ListModel {
            ListElement {foo: "Item 1"} ListElement {foo: "Item 2"}
            ListElement {foo: "Item 3"} ListElement {foo: "Item 4"}
            ListElement {foo: "Item 5"} ListElement {foo: "Item 6"}
            ListElement {foo: "Item 7"} ListElement {foo: "Item 8"}
          }
          // delegate: Rectangle {
          delegate: Label {
            required property string foo
            text: foo
            // Layout.fillWidth: true
            // Layout.preferredHeight: 30
            // Text {
            //   text: parent.foo
            // }
          }
        }
      }

      ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.alignment: Qt.AlignTop

        TabBar {
          wheelEnabled: true
          TabButton { text: "Diff 1" }
          TabButton { text: "Diff 2" }
          TabButton { text: "+" }
        }

        Rectangle {
          // Layout.preferredHeight: 200
          Layout.fillWidth: true
          Layout.fillHeight: true
          radius: 10.
          color: "red"
          Label {
            anchors.centerIn: parent
            text: "Chart preview will be shown here"
          }
        }
        Label {
          text: "Editor state will be shown here"
        }
        CheckBox {
          checked: true
          text: "Downscroll"
        }
        SpinBox {
          from: 100.
          to: 1000.
          value: 700.
          stepSize: 25.
          wheelEnabled: true
        }
        Rectangle {
          Layout.fillWidth: true
          // Layout.fillHeight: true
          color: "lightblue"
          Layout.preferredHeight: 200
          Label {
            anchors.centerIn: parent
            text: "scrollable note table will be shown here"
          }
        }

      }
    }
  }
}
