pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music

FullScreenWindow {
    id: root
    visible: true
    windowTitle: "HX.Music 歌词浮窗"
    // color: "#05990099"
    
    // 是否锁定
    property bool locked: false

    // 是否显示控制栏(包括鼠标悬浮后显示的解锁按钮)
    property bool showControls: !locked

    // 外部接口: 锁/解锁控制
    function lock() {
        locked = true;
        Qt.callLater(() => {
            const posInRoot = windowItemRef.lockButtonRef.mapToItem(windowItemRef.delegateRoot, 0, 0);
            WindowMaskUtil.clear(root);
            WindowMaskUtil.addControlRect(
                posInRoot.x, posInRoot.y,
                windowItemRef.lockButtonRef.width,
                windowItemRef.lockButtonRef.height
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

    windowItem: Item {
        id: delegateRoot
        width: 800
        height: 200
        property alias lockButtonRef: lockButton
        property alias delegateRootRef: delegateRoot

        Rectangle {
            visible: root.showControls
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
                onClicked: MusicController.prev()
            }
            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: MusicController.isPlaying ? "qrc:/icons/pause.svg"
                                                      : "qrc:/icons/play.svg"
                }
                onClicked: MusicController.togglePause()
            }
            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: "qrc:/icons/next.svg"
                }
                onClicked: MusicController.next()
            }
            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: "qrc:/icons/back.svg"
                }
                onClicked: SignalBusSingleton.lyricAddOffset(-100)
            }
            Button {
                visible: root.showControls
                background: Rectangle { color: "transparent" }
                Image {
                    anchors.fill: parent
                    source: "qrc:/icons/enter.svg"
                }
                onClicked: SignalBusSingleton.lyricAddOffset(100)
            }
            Button {
                id: lockButton
                visible: root.showControls || mouseArea._isHover
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
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            property bool _isHover: false
            hoverEnabled: true

            // 延迟退出时间 (毫秒)
            property int hoverDelay: 1500

            Timer {
                id: exitTimer
                interval: mouseArea.hoverDelay
                repeat: false;
                onTriggered: {
                    mouseArea._isHover = false;
                }
            }

            onEntered: {
                // 停止退出定时器, 保证回到 hover 状态
                if (exitTimer.running) {
                    exitTimer.stop();
                }
                _isHover = true;
            }

            onExited: {
                // 启动退出定时器, 延迟取消 hover 状态
                if (exitTimer.running) {
                    exitTimer.stop();
                }
                exitTimer.start();
            }
        }
    }
    Rectangle {
        id: selfSize
        anchors.fill: parent
        color: "transparent"
    }
}
