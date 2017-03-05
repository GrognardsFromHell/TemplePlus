import QtQuick 2.0
import TemplePlus 1.0
import "../Common"

Item {
    width: 800
    height: 600

    property var items
    signal close

    Component.onCompleted: {
        items = cinematics

        // Add 50 test items if in creator mode
        if (qtCreator) {
            var fakeItems = [];
            for (var i = 0; i < 50; i++) {
                fakeItems.push({id: i, name: "Cinematic " + i});
            }
            items = fakeItems;
        }
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
            Label {
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

        ScrollBar {
            id: scrollbar
            x: 256
            y: cinematicsList.visibleArea.heightRatio * cinematicsList.contentHeight
            anchors.top: cinematicsList.top
            anchors.bottom: cinematicsList.bottom
            max: Math.max(0, cinematicsList.contentHeight - cinematicsList.height)

            onUpdate: {
                cinematicsList.contentY = value;
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
