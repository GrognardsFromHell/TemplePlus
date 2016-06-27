import QtQuick 2.0

Item {
    property alias label : label.text
    property bool showDivider : !Positioner.isLastItem
    height: 35
    width: parent.width
    Text {
        id: label
        text: "Option 1"
        color: "white"
    }
    Rectangle {
        color: "#5d5d5d"
        width: parent.width
        height: 1
        anchors.bottom: parent.bottom
        visible: showDivider
    }
}
