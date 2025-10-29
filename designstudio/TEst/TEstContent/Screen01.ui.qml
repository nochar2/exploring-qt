/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// was Rectangle before

Rectangle {
    id: rectangle
    width: 800
    height: 600

    color: Constants.backgroundColor

    RowLayout {

        Text {
            id: text1
            x: 246
            y: 117
            width: 199
            height: 70
            text: qsTr("Here is a bunch of text...")
            font.pixelSize: 22
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
    }
}
