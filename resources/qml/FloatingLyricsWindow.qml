import QtQuick
import QtQuick.Controls
import QtQuick.Window

Window {
    id: floatWin
    title: "歌词浮窗[Wayland置顶]"
    width: 500
    height: 180

    flags: Qt.FramelessWindowHint       // 无边框
         | Qt.WindowStaysOnTopHint      // 窗口顶置
         | Qt.Tool                      // 表述为浮动窗口
         | Qt.WindowDoesNotAcceptFocus  // 窗口不接受焦点
         | Qt.WindowTransparentForInput // 鼠标穿透

    transientParent: null
    color: "transparent" // 窗口透明

    Rectangle {
        anchors.fill: parent
        color: "#222222cc" // 半透明背景
        radius: 6
        border.color: "#888"
        border.width: 1

        // 模拟歌词
        Text {
            anchors.centerIn: parent
            text: "ASS 歌词占位"
            font.pixelSize: 20
            color: "white"
        }

        // 模拟“关闭”按钮
        Button {
            text: "关闭"
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            onClicked: floatWin.visibility = "Hidden"
        }
    }
}
