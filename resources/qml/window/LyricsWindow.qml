pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music

FullScreenWindow {
    id: root
    windowTitle: "HX.Music 歌词浮窗"

    // 是否锁定
    property bool locked: false

    // 是否显示锁 (悬浮情况)
    property bool showUnlock: false

    // 是否显示控制栏(包括鼠标悬浮后显示的解锁按钮)
    property bool showControls: !locked

    // 外部接口: 锁/解锁控制
    function lock() {
        locked = true;
        root.closeMask = true;
        LyricController.isLocked = true;
        Qt.callLater(() => {
            const posInRoot = windowItemRef.lockButtonRef.mapToItem(fillRect, 0, 0);
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
        root.closeMask = false;
        LyricController.isLocked = false;
        Qt.callLater(() => {
            WindowMaskUtil.clear(root);
            root.updateMask();
        });
    }

    Rectangle {
        id: fillRect
        anchors.fill: parent
        color: "transparent"
    }

    initX: LyricController.windowX
    initY: LyricController.windowY
    initWidth: LyricController.windowWidth
    initHeight: LyricController.windowHeight
    windowItem: Item {
        id: delegateRoot
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
                visible: root.showControls || root.showUnlock
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

    Timer {
        id: exitTimer
        // 延迟退出时间 (毫秒)
        interval: 1500
        repeat: false;
        onTriggered: {
            root.showUnlock = false;
        }
    }

    onIsHoverChanged: {
        if (root.isHover) {
            if (exitTimer.running) {
                exitTimer.stop();
            }
            root.showUnlock = true;
        } else {
            exitTimer.start();
        }
    }

    onItemXChanged: function(v: int) {
        LyricController.windowX = v;
    }

    onItemYChanged: function(v: int) {
        LyricController.windowY = v;
    }


    onItemWidthChanged: function(v: int) {
        if (v === 0)
            return;
        LyricController.windowWidth = v;
    }

    onItemHeightChanged: function(v: int) {
        if (v === 0)
            return;
        LyricController.windowHeight = v;
    }

    onVisibleChanged: function(v: bool) {
        if (v) {
            LyricController.renderAFrameInstantly();
        }
    }
}
