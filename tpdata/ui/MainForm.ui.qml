import QtQuick 2.8

Item {
    id: item1
    property alias mouseArea: mouseArea

    width: 800
    height: 600

    MouseArea {
        id: mouseArea
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.fill: parent
    }

    Image {
        y: 30
        anchors.horizontalCenter: parent.horizontalCenter
        source: 'tio:///art/interface/mainmenu_ui/MainMenu_Title.img'
    }

    Column {
        y: 292
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        anchors.right: parent.right
        anchors.left: parent.left
        Repeater {
            id: repeater
            model: menuItems
            delegate: MainMenuButton {
                text: model.name
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
