import QtQuick 2.0
import TemplePlus 1.0

Item {
    id: root
    width: normalImg.sourceSize.width
    height: normalImg.sourceSize.height

    property alias mouseArea : mouse
    property alias text : label.text
    property bool disabled : false

    property alias source : normalImg.source
    property alias hoverSource : hoverImg.source
    property alias pressedSource : pressedImg.source
    property alias disabledSource : disabledImg.source

    onSourceChanged: {
        if (hoverImg.source == "") {
            hoverImg.source = source;
        }
        if (pressedImg.source == "") {
            pressedImg.source = source;
        }
        if (disabledImg.source == "") {
            disabledImg.source = source;
        }
    }

    Item {
        visible: !disabled
        Image {
            id: normalImg
            visible: !mouse.pressed || !mouse.containsMouse
        }

        Image {
            id: hoverImg
            visible: !mouse.pressed && mouse.containsMouse
        }

        Image {
            id: pressedImg
            visible: mouse.pressed && mouse.containsMouse
        }
    }
    Image {
        id: disabledImg
        width: 112
        height: 22
        source: "tio:///art/interface/GENERIC/Disabled_Normal.tga"
        visible: disabled
    }

    Label {
        id: label
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
