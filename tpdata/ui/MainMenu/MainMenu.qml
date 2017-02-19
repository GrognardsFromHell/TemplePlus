
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
                    action("start_game_normal");
                }
            },
            {
                text: qsTr("mainmenu:11"), // ironman
                onClick: function() {
                    action("start_game_ironman");
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
        id: ingameMenuNormal
        anchors.fill: parent
        visible: page == 2

        focus: true
        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                action("ingame_close");
            }
        }

        buttons: [
            {
                text: qsTr("mainmenu:20"), // load game
                onClick: function() {
                    action("ingame_normal_load_game");
                }
            },
            {
                text: qsTr("mainmenu:21"), // save game
                onClick: function() {
                    action("ingame_normal_save_game");
                }
            },
            {
                text: qsTr("mainmenu:22"), // return to game
                onClick: function() {
                    action("ingame_close");
                }
            },
            {
                text: qsTr("mainmenu:23"), // quit
                onClick: function() {
                    action("ingame_normal_quit");
                }
            }
        ]

    }

    MainMenuPage {
        id: ingameMenuIronman
        anchors.fill: parent
        visible: page == 3

        focus: true
        Keys.onPressed: {
            if (event.key === Qt.Key_Escape) {
                action("ingame_close");
            }
        }

        buttons: [
            {
                text: qsTr("mainmenu:30"), // load game
                onClick: function() {
                    action("ingame_close");
                }
            },
            {
                text: qsTr("mainmenu:31"), // save game & quit
                onClick: function() {
                    action("ingame_ironman_save_and_quit");
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
                    action("options_show");
                }
            },
            {
                text: qsTr("mainmenu:41"), // cinematics
                onClick: function() {
                    action("options_view_cinematics");
                }
            },
            {
                text: qsTr("mainmenu:42"), // credits
                onClick: function() {
                    action("options_credits");
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
