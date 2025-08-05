import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: mainWin
    width: 640
    height: 480
    visible: true
    title: "歌词浮窗测试"

    property var floatingWin: null

    Component {
        id: floatingLyricsComponent

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

                // 模拟“关闭”按钮（可删除）
                Button {
                    text: "关闭"
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    anchors.margins: 10
                    onClicked: floatWin.visibility = "Hidden"
                }

                // 模拟鼠标穿透：禁用 MouseArea（部分环境有效）
                MouseArea {
                    anchors.fill: parent
                    enabled: false
                }
            }
        }
    }

    // 主窗口按钮
    Button {
        text: "歌词浮窗"
        anchors.centerIn: parent
        onClicked: {
            if (!floatingWin) {
                floatingWin = floatingLyricsComponent.createObject(null)
            }
            floatingWin.visibility = "Windowed"
            floatingWin.raise()
        }
    }
}
