import QtQuick 2.6
import QtQuick.Controls 1.4
import "../common" as Common

Item {

    // Center in parent container
    anchors.centerIn: parent
    width: 518
    height: 499

    Text {
        color: "#ffffff"
        anchors.centerIn: parent
        text: "Hello World"
    }

    Image {
        x: 0
        y: 0
        source: "background.png"
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        ScrollView {
            id: scrollView
            x: 30
            y: 47
            width: 461
            height: 390
            style: Common.ToEEScrollViewStyle {
            }
            Column {
                width: scrollView.viewport.width
                OptionsItem {
                    label: "Option 1"
                }
                OptionsItem {
                    label: "Option 2"
                }
                OptionsItem {
                    label: "Option 3"
                }
                OptionsItem {
                    label: "Option 4"
                }
                OptionsItem {
                    label: "Option 5"
                }
                OptionsItem {
                    label: "Option 6"
                }
                OptionsItem {
                    label: "Option 7"
                }
                OptionsItem {
                    label: "Option 8"
                }
                OptionsItem {
                    label: "Option 9"
                }
                OptionsItem {
                    label: "Option 10"
                }
                OptionsItem {
                    label: "Option 10"
                }
                OptionsItem {
                    label: "Option 11"
                }
                OptionsItem {
                    label: "Option 12"
                }
                OptionsItem {
                    label: "Option 13"
                }
            }
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
        }

        Common.StandardButton {
            id: okButton
            x: 133
            y: 454
            text: "Accept"
            source: "../common/Accept_Normal.png"
            hoverSource: "../common/Accept_Hover.png"
            pressedSource: "../common/Accept_Pressed.png"
        }

        Common.StandardButton {
            id: cancelButton
            x: 274
            y: 454
            text: "Cancel"
            source: "../common/Decline_Normal.png"
            hoverSource: "../common/Decline_Hover.png"
            pressedSource: "../common/Decline_Pressed.png"
        }
    }

}
