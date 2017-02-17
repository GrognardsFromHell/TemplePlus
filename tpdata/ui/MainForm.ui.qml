import QtQuick 2.8
import TemplePlus 1.0

Item {
    id: item1
    property alias mouseArea: mouseArea

    width: 360
    height: 360

    MouseArea {
        id: mouseArea
        anchors.rightMargin: 0
        anchors.bottomMargin: 0
        anchors.fill: parent
    }

    Image {
        y: 30
        anchors.horizontalCenter: parent.horizontalCenter
       // source: 'tio:///art/interface/mainmenu_ui/MainMenu_Title.img'
    }

	Column {
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 50

		MainMenuButton {
			text: "New Game"
		}		
	}
	
    Column {
        y: 292
        height: 200
        anchors.right: parent.right
        anchors.left: parent.left
        Repeater {
            id: repeater
            model: menuItems
            delegate: Text {
                text: model.name
                width: contentWidth
                height: 40
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                color: "white"
            }
        }
    }
}
