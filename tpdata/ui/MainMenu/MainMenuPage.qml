
import QtQuick 2.8

Item {
    property var buttons : []
    property bool showLogo : true

    width: 800
    height: 600

    // Prevent click-through for main menu
    MouseArea {
        anchors.fill: parent
    }

    Image {
        y: 30
        anchors.horizontalCenter: parent.horizontalCenter
        source: 'tio:///art/interface/mainmenu_ui/MainMenu_Title.img'
        visible: showLogo
    }

    Column {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 50
        anchors.right: parent.right
        anchors.left: parent.left
        Repeater {
            model: buttons
            delegate: MainMenuButton {
                text: modelData.text
                anchors.horizontalCenter: parent.horizontalCenter
                mouseArea.onClicked: buttons[index].onClick()
            }
        }
    }
}
