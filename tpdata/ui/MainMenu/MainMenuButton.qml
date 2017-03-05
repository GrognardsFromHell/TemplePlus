import QtQuick 2.8
import TemplePlus 1.0
import "../Common"

Item {
    property alias mouseArea : mouseArea
    property string text : ""
	width: normalText.implicitWidth
	height: normalText.implicitHeight
	
    LegacyText {
		id: normalText
		text: parent.text
		visible: !mouseArea.containsMouse && !mouseArea.containsPress
	}

    LegacyText {
		text: parent.text
		color: "#01FFFF"
		gradientColor: "#01D0FF"
		visible: mouseArea.containsMouse && !mouseArea.containsPress
	}

    LegacyText {
		text: parent.text
		color: "#EB1510"
		gradientColor: "#DA5B61"
		visible: mouseArea.containsPress
	}
	
	MouseArea {
		id: mouseArea
		anchors.fill: parent
		hoverEnabled: true
        onEntered: playSound(3010)
        onExited: playSound(3011)
        onPressed: playSound(3012)
        onClicked: playSound(3013)
	}
	
}
