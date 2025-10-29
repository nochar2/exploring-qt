import QtQuick

Item {
  id: root
  width: 400
  height: 400

  Rectangle {
    id: first
    x: 0; y: 0
    width: root.width
    height: 200
    color: "lightblue"

    Rectangle {
      x: 50; y: 50
      width: 50
      height: 50
      color: "magenta" // second.color
    }
  }
  Rectangle {
    id: second
    x: 0; y: 200
    width: root.width
    height: 200
    color: "green"

    Rectangle {
      x: 80; y: -30
      width: 100
      height: 50
      color: "yellow"
    }
  }
  
}
