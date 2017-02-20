import QtQuick 2.0
import TemplePlus 1.0

Item {
    width: 800
    height: 600

    property var items
    signal close

    Component.onCompleted: {
        items = cinematics
        console.log("Cinematics: ", JSON.stringify(items))
    }

    Item {
        width: image.width
        height: image.height
        anchors.centerIn: parent

        Image {
            id: image
            source: "tio:///art/interface/pc_creation/meta_backdrop.img"
            width: 291
            height: 357
        }

        Component {
            id: cinematicSelector
            Text {
                height: 25
                verticalAlignment: Text.AlignVCenter
                anchors.horizontalCenter: parent.horizontalCenter
                text: items[index].name
                color: ListView.isCurrentItem ? "red" : "white"
                MouseArea {
                    anchors.fill: parent
                    onClicked: cinematicsList.currentIndex = index
                }
            }
        }

        ListView {
            id: cinematicsList
            x: 22
            y: 71
            width: scrollbar.x - 22
            height: 219
            model: items
            delegate: cinematicSelector
            focus: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            onContentYChanged: {
                scrollbar.value = contentY
            }
        }

        Item {
            id: scrollbar
            x: 256
            y: cinematicsList.visibleArea.heightRatio * cinematicsList.contentHeight
            width: scrollbarTop.width
            anchors.top: cinematicsList.top
            anchors.bottom: cinematicsList.bottom

            property real value : 0
            property real max: Math.max(0, cinematicsList.contentHeight - cinematicsList.height)
            onValueChanged: {
                var percentage = value / max;
                handle.y = percentage * (track.height - handle.height);
                if (dragArea.drag.active) {
                    cinematicsList.contentY = value;
                }
            }

            Image {
                id: scrollbarTop
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Arrow_Top.tga"
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

            Image {
                id: scrollbarBottom
                anchors.bottom: parent.bottom
                source: "tio:///art/interface/SCROLLBAR/ScrollBar_Arrow_Bottom.tga"
            }
        }

        StandardButton {
            x: 28
            y: 306
            text: "View"
            disabled: !cinematicsList.currentItem
            mouseArea.onClicked: {
                playCinematic(cinematics[cinematicsList.currentIndex])
            }
        }

        StandardButton {
            x: 152
            y: 306
            text: "Cancel"
            disabled: false
            cancel: true
            mouseArea.onClicked: close()
        }
    }
}
