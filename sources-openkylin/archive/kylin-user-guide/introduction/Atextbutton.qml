import QtQuick 2.0

Rectangle {

      id: root
      visible: true

      property alias text: label.text
      signal clicked

      width: 116; height: 26
      color: "lightsteelblue"

      Text {
          id: label
          anchors.centerIn: parent
          text: "Start"
      }
      MouseArea {
          anchors.fill: parent
          onClicked: {
              root.clicked()
          }
      }
}
