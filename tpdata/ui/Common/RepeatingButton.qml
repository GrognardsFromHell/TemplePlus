import QtQuick 2.0

Button {
    id: root

    Timer {
        running: root.mouseArea.pressed && root.mouseArea.containsMouse
        repeat: true
        interval: 250
        onTriggered: root.mouseArea.clicked(0)
    }
}
