import QtQuick
// import QtQuick.Controls

Item {
  id: itm
  // this is stored
  required property string color
  required property string label
  property int radius: 10


  Rectangle {
    id: rect
    width: itm.width
    height: itm.height
    radius: itm.radius
    
    // this is computed
    color: itm.color
    ColorAnimation on color {
      id: animation
      from: "white"
      to: itm.color
      duration: 500
    }


    MouseArea {
      anchors.fill: parent
      onClicked: { animation.stop(); animation.start(); }
    }
    Text {
      anchors.centerIn: parent
      horizontalAlignment: Text.AlignHCenter; verticalAlignment:   Text.AlignVCenter;
      font.pointSize: 40
      text: itm.label
    }
  }

  Rectangle {
    width: itm.width
    height: itm.height
    radius: itm.radius
    
    gradient: Gradient {
      GradientStop { position: 0.0; color: "transparent"; } 
      // TODO: there's also Qt.darker you could use it somehow
      GradientStop { position: 3.0; color: "black"; } 
    }
  }
}
