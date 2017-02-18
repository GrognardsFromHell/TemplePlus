
import QtQuick 2.8

Item {

    width: 800
    height: 600

    signal action(string type)

    property int page : 0

    MainMenuPage {
        id: welcomePage
        anchors.fill: parent
        visible: page == 0
        buttons: [
            {
                text: qsTr("mainmenu:0"), // new game
                onClick: function() {
                    page = 1;
                }
            },
            {
                text: qsTr("mainmenu:1"), // load game
                onClick: function() {
                    action("load_game");
                }
            },
            {
                text: qsTr("mainmenu:2"), // tutorial
                onClick: function() {
                    action("tutorial");
                }
            },
            {
                text: qsTr("mainmenu:3"), // options
                onClick: function() {
                    page = 4;
                }
            },
            {
                text: qsTr("mainmenu:4"), // quit
                onClick: function() {
                    action("quit");
                }
            }
        ]
    }

    MainMenuPage {
        id: difficultySelectPage
        anchors.fill: parent
        visible: page == 1
        buttons: [
            {
                text: qsTr("mainmenu:10"), // normal
                onClick: function() {

                }
            },
            {
                text: qsTr("mainmenu:11"), // ironman
                onClick: function() {

                }
            },
            {
                text: qsTr("mainmenu:12"), // exit
                onClick: function() {
                    page = 0;
                }
            }
        ]
    }

    MainMenuPage {
        id: optionsPage
        anchors.fill: parent
        visible: page == 4
        buttons: [
            {
                text: qsTr("mainmenu:40"), // game options
                onClick: function() {
                    action("options");
                }
            },
            {
                text: qsTr("mainmenu:41"), // cinematics
                onClick: function() {
                    // TODO: cinematics
                }
            },
            {
                text: qsTr("mainmenu:42"), // credits
                onClick: function() {
                    action("credits");
                }
            },
            {
                text: qsTr("mainmenu:43"), // back
                onClick: function() {
                    page = 0;
                }
            }
        ]
    }

}
