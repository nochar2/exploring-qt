import QtQuick

Item {
    ListModel {
        id: colorModel
        ListElement { color: "red" }
        ListElement { color: "green" }
        ListElement { color: "blue" }
    }

    Row {
        anchors.centerIn: parent
        Repeater {
            // model: 10
            model: colorModel
            Rectangle {
                width: 80
                height: 80
                color: model.color
                // Text {
                //     text: model
                // }
            }
        }
    }
}
