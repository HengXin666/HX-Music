import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: floatWin
    width: 500
    height: 180
    visible: false
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
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
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        onClicked: floatWin.visible = false
    }
}
