pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music

BorderlessWindow {
    id: root
    width: 800
    height: 200
    visible: true
    title: "HX.Music 歌词浮窗 [Wayland置顶]"
    titleBar: null
    showBorder: false
    
    // 是否锁定
    property bool locked: false

    // 是否显示控制栏(包括鼠标悬浮后显示的解锁按钮)
    property bool showControls: !locked

    // 外部接口: 锁/解锁控制
    function lock() {
        locked = true;
        Qt.callLater(() => {
            const posInRoot = delegateRef.lockButtonRef.mapToItem(delegateRef.delegateRoot, 0, 0);
            WindowMaskUtil.addControlRect(
                posInRoot.x, posInRoot.y,
                delegateRef.lockButtonRef.width,
                delegateRef.lockButtonRef.height
            );
            WindowMaskUtil.setMask(root);
        });
    }
    function unlock() {
        locked = false;
        Qt.callLater(() => {
            WindowMaskUtil.clear(root);
        });
    }

    flags: Qt.FramelessWindowHint
         | Qt.WindowStaysOnTopHint
         | Qt.Tool
        //  | (locked ? Qt.WindowDoesNotAcceptFocus | Qt.WindowTransparentForInput : Qt.NoItemFlags)

    color: "transparent"

    delegate: Item {
        id: delegateRoot
        property alias lockButtonRef: lockButton
        property alias delegateRootRef: delegateRoot

        anchors.fill: parent
        Rectangle {
            // visible: mouseArea._isHover
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
            spacing: 10
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                margins: 12
            }

            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: "qrc:/icons/previous.svg"
                }
                onClicked: musicController.prev()
            }
            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: musicController.isPlaying ? "qrc:/icons/pause.svg"
                                                      : "qrc:/icons/play.svg"
                }
                onClicked: musicController.togglePause()
            }
            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: "qrc:/icons/next.svg"
                }
                onClicked: musicController.next()
            }
            Button {
                id: lockButton
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: root.showControls ? "qrc:/icons/lock.svg" : "qrc:/icons/unlock.svg"
                }
                onClicked: root.showControls ? root.lock() : root.unlock()
            }
        }
        Component.onCompleted: {
            // 更新歌词
            LyricController.updateLyriced.connect(() => {
                lyricImage.source = `image://musicLyric?f=${Date.now()}`;
            });
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        property bool _isHover: false
        hoverEnabled: true // 必须开启, 否则不会触发 onEntered/onExited

        onEntered: {
            _isHover = true;
            console.log("鼠标进入")
        }

        onExited: {
            _isHover = false;
            console.log("鼠标离开")
        }
    }
}
