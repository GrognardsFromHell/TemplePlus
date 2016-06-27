import QtQuick 2.0
import QtQuick.Controls.Styles 1.4

ScrollViewStyle {
    frame: Item {}
    handleOverlap: 0
    padding.left: 0
    padding.top: 0
    padding.right: 0
    padding.bottom: 0
    decrementControl: Item {
        width: scrollBtnUp.width
        height: scrollBtnUp.height - 1
        Image {
            id: scrollBtnUp
            source: 'scrollbar/ScrollBar_Arrow_Top.png'
            visible: !styleData.pressed
        }
        Image {
            source: 'scrollbar/ScrollBar_Arrow_Top_Click.png'
            visible: styleData.pressed
        }
    }
    scrollBarBackground: Image {
        width: 15
        fillMode: Image.TileVertically
        source: 'scrollbar/ScrollBar_Empty.png'
    }
    handle: Item {
        width: handleBg.width
        Image {
            id: handleBg
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            fillMode: Image.Stretch
            source: 'scrollbar/ScrollBar_Fill.png'
        }
        Image {
            id: handleTop
            height: 3
            fillMode: Image.Pad
            anchors.top: parent.top
            source: 'scrollbar/ScrollBar_Fill_Top.png'
        }
        Image {
            id: handleBottom
            height: 3
            fillMode: Image.Pad
            anchors.bottom: parent.bottom
            source: 'scrollbar/ScrollBar_Fill_Bottom.png'
        }
    }
    incrementControl: Item {
        width: scrollBtnDown.width
        height: scrollBtnDown.height - 1
        Image {
            y: -1
            id: scrollBtnDown
            source: 'scrollbar/ScrollBar_Arrow_Bottom.png'
            visible: !styleData.pressed
        }
        Image {
            y: -1
            source: 'scrollbar/ScrollBar_Arrow_Bottom_Click.png'
            visible: styleData.pressed
        }
    }
}
