import QtQuick 2.8
import TemplePlus 1.0
import "../Common"

Item {

    width: 516
    height: 497

    Image {
        id: background
        source: "tio:///art/interface/options_ui/options_background.img"
    }

    DialogTitle {
        y: 10
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("options:6")
    }

    StandardButton {
        x: 133
        y: 454
        mouseArea.onClicked: console.log(background.width)
        text: qsTr("options:4")
    }

    StandardButton {
        x: 274
        y: 454
        cancel: true
        text: qsTr("options:5")
    }
}
