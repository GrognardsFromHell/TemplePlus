import QtQuick 2.0
import TemplePlus 1.0

Item {
    id: root
    width: 112
    height: 22

    property alias mouseArea : mouse
    property string text : "Text"
    property bool disabled : false
    property bool cancel : false

    Item {
        visible: !disabled && !cancel
        Image {
            width: 112
            height: 22
            source: "tio:///art/interface/GENERIC/Accept_Normal.tga"
            visible: !mouse.pressed && !mouse.containsMouse
        }

        Image {
            width: 112
            height: 22
            source: "tio:///art/interface/GENERIC/Accept_Pressed.tga"
            visible: mouse.pressed
        }

        Image {
            width: 112
            height: 22
            source: "tio:///art/interface/GENERIC/Accept_Hover.tga"
            visible: !mouse.pressed && mouse.containsMouse
        }
    }
    Item {
        visible: !disabled && cancel
        Image {
            width: 112
            height: 22
            source: "tio:///art/interface/GENERIC/Decline_Normal.tga"
            visible: !mouse.pressed && !mouse.containsMouse
        }

        Image {
            width: 112
            height: 22
            source: "tio:///art/interface/GENERIC/Decline_Pressed.tga"
            visible: mouse.pressed
        }

        Image {
            width: 112
            height: 22
            source: "tio:///art/interface/GENERIC/Decline_Hover.tga"
            visible: !mouse.pressed && mouse.containsMouse
        }
    }

    Image {
        width: 112
        height: 22
        source: "tio:///art/interface/GENERIC/Disabled_Normal.tga"
        visible: disabled
    }

    Text {
        anchors.fill: parent
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        text: root.text
        color: "white"
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
    }
}
