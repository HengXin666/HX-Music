import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: mainWin
    width: 640
    height: 480
    visible: true
    title: "歌词浮窗测试 [Wayland置顶]"

    property var floatingWin: null

    Component {
        id: floatingLyricsComponent
        Window {
            id: floatWin
            width: 500
            height: 180
            visible: false
            flags: Qt.Window | Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus
            title: "浮动歌词"

            Rectangle {
                anchors.fill: parent
                color: "#222222cc"
                radius: 6
                border.color: "#888"
                border.width: 1
            }

            Text {
                anchors.centerIn: parent
                text: "ASS 歌词占位"
                font.pixelSize: 20
                color: "white"
            }

            Button {
                text: "关闭"
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: 10
                onClicked: floatWin.visible = false
            }
        }
    }

    Button {
        text: "歌词"
        anchors.centerIn: parent
        onClicked: {
            if (!floatingWin) {
                floatingWin = floatingLyricsComponent.createObject(null)
            }
            floatingWin.visible = true
            floatingWin.raise()
        }
    }
}
