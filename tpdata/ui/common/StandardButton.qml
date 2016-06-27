import QtQuick 2.0

Item {

    property alias source: normal.source
    property alias hoverSource: hovered.source
    property alias pressedSource: pressed.source
    property alias text: text.text

    width: normal.width
    height: normal.height

    Image {
        id: normal
        visible: !mouseArea.containsMouse
    }

    Image {
        id: hovered
        visible: mouseArea.containsMouse && !mouseArea.pressed
    }

    Image {
        id: pressed
        visible: mouseArea.containsMouse && mouseArea.pressed
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Text {
        id: text
        color: "white"
        font.bold: true
        anchors.centerIn: parent
    }

}
