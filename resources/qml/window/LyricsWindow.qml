import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music

Window {
    id: floatWin
    width: 800
    height: 200
    visible: true
    title: "HX.Music 歌词浮窗 [Wayland置顶]"

    // 是否锁定
    property bool locked: false

    // 是否显示控制栏(包括鼠标悬浮后显示的解锁按钮)
    property bool showControls: !locked

    // 外部接口: 锁/解锁控制
    function lock() {
        locked = true
    }
    function unlock() {
        locked = false
    }

    // C++侧: 歌词控制器
    LyricController {
        id: lyricController
    }

    MusicController {
        id: musicController
    }

    flags: Qt.FramelessWindowHint
         | Qt.WindowStaysOnTopHint
         | Qt.Tool
         | (locked ? Qt.WindowDoesNotAcceptFocus | Qt.WindowTransparentForInput : Qt.NoItemFlags)

    color: "transparent"

    Rectangle {
        visible: floatWin.showControls
        anchors.fill: parent
        color: "#3f000000"
    }

    Image {
        id: lyricImage
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        smooth: true
        cache: false
        source: ""
    }

    RowLayout {
        id: controlBar
        visible: floatWin.showControls
        spacing: 10
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            margins: 12
        }

        Button {
            Image {
                anchors.fill: parent
                source: "qrc:/icons/previous.svg"
            }
            onClicked: musicController.prev()
        }
        Button {
            Image {
                anchors.fill: parent
                source: "qrc:/icons/pause.svg"
            }
            onClicked: musicController.togglePause()
        }
        Button {
            Image {
                anchors.fill: parent
                source: "qrc:/icons/next.svg"
            }
            onClicked: musicController.next()
        }
        Button {
            Image {
                anchors.fill: parent
                source: "qrc:/icons/lock.svg"
            }
            onClicked: floatWin.lock()
        }
    }

    Component.onCompleted: {
        // 更新歌词
        lyricController.updateLyriced.connect(() => {
            lyricImage.source = `image://musicLyric?f=${Date.now()}`;
        });
    }
}
