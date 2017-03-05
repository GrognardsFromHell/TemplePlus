import QtQuick 2.0

Item {
    id: scrollbar
    width: scrollbarTop.width

    property real value : 0
    property real max : 100

    signal update(int value);

    onValueChanged: {
        var percentage = value / max;
        handle.y = percentage * (track.height - handle.height);
        if (dragArea.drag.active) {
            update(value);
        }
    }

    RepeatingButton {
        id: scrollbarTop
        source: "tio:///art/interface/SCROLLBAR/ScrollBar_Arrow_Top.tga"
        pressedSource: "tio:///art/interface/SCROLLBAR/ScrollBar_Arrow_Top_Click.tga"
        mouseArea.onClicked: {
            scrollbar.value = Math.max(0, scrollbar.value - scrollbar.max * 0.05)
            scrollbar.update(scrollbar.value);
        }
    }

    Image {
        anchors.top: scrollbarTop.bottom
        anchors.bottom: scrollbarBottom.top
        source: "tio:///art/interface/SCROLLBAR/ScrollBar_Empty.tga"
    }

    Item {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: scrollbarTop.bottom
        anchors.bottom: scrollbarBottom.top

        Item {
            id: handle
            height: 60
            anchors.left: parent.left
            anchors.right: parent.right
            onYChanged:  {
                if (dragArea.drag.active) {
                    var maxY = parent.height - height;
                    var percentage = y / maxY;
                    scrollbar.value = percentage * scrollbar.max;
                }
            }

            Image {
                id: fillTop
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Fill_Top.tga"
                visible: !dragArea.pressed
            }
            Image {
                anchors.top: fillTop.bottom
                anchors.bottom: fillBottom.top
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Fill.tga"
                visible: !dragArea.pressed
            }
            Image {
                id: fillBottom
                anchors.bottom: parent.bottom
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Fill_Bottom.tga"
                visible: !dragArea.pressed
            }

            Image {
                id: fillTopDragging
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Fill_Top_Click.tga"
                visible: dragArea.pressed
            }
            Image {
                anchors.top: fillTopDragging.bottom
                anchors.bottom: fillBottomDragging.top
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Fill_Click.tga"
                visible: dragArea.pressed
            }
            Image {
                id: fillBottomDragging
                anchors.bottom: parent.bottom
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Fill_Bottom_Click.tga"
                visible: dragArea.pressed
            }

            MouseArea {
                id: dragArea
                anchors.fill: handle
                drag.target: handle
                drag.axis: Drag.YAxis
                drag.minimumY: 0
                drag.maximumY: track.height - handle.height
            }
        }
    }

    RepeatingButton {
        id: scrollbarBottom
        anchors.bottom: parent.bottom
        source: "tio:///art/interface/SCROLLBAR/ScrollBar_Arrow_Bottom.tga"
        pressedSource: "tio:///art/interface/SCROLLBAR/ScrollBar_Arrow_Bottom_Click.tga"
        mouseArea.onClicked: {
            scrollbar.value = Math.min(scrollbar.max, scrollbar.value + scrollbar.max * 0.05);
            scrollbar.update(scrollbar.value);
        }
    }
}
