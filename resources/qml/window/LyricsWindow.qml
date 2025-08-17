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

    // 是否允许缩放
    allowZooming: !LyricController.isFullScreen

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

    signal reqClose();

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
            y: 32
            height: parent.height - 32
            anchors {
                left: parent.left
                right: parent.right
            }
            fillMode: Image.PreserveAspectFit
            smooth: true
            cache: false
            source: ""
        }

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
            }
            color: "transparent"
            y: 0
            height: 32
            z: 10
            RowLayout {
                id: controlBar
                spacing: 10
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.margins: 8
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/previous.svg"
                    onClicked: MusicController.prev()
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url:  MusicController.isPlaying ? "qrc:/icons/pause.svg"
                                                    : "qrc:/icons/play.svg"
                    onClicked: MusicController.togglePause()
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/next.svg"
                    onClicked: MusicController.next()
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/reset.svg"
                    onClicked: MusicController.setPosition(0)
                }
                Item {
                    Layout.preferredWidth: 32
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/back.svg"
                    onClicked: SignalBusSingleton.lyricAddOffset(-100)
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/enter.svg"
                    onClicked: SignalBusSingleton.lyricAddOffset(100)
                }
                MusicActionButton {
                    id: lockButton
                    visible: root.showControls || root.showUnlock
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: root.showControls ? "qrc:/icons/lock.svg" : "qrc:/icons/unlock.svg"
                    onClicked: root.showControls ? root.lock() : root.unlock()
                }
                Item {
                    Layout.preferredWidth: 32
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/center.svg"
                    onClicked: {
                        LyricController.windowX = (root.width - LyricController.windowWidth) >> 1;
                        root.synchronousCoordinates();
                    }
                }
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: LyricController.isFullScreen ? "qrc:/icons/restore.svg" : "qrc:/icons/up.svg"
                    onClicked: {
                        // 设置全屏
                        if (LyricController.isFullScreen) {
                            LyricController.windowX = LyricController.maeWindowX;
                            LyricController.windowY = LyricController.maeWindowY;
                            LyricController.windowWidth = LyricController.maeWindowWidth;
                            LyricController.windowHeight = LyricController.maeWindowHeight;
                        } else {
                            LyricController.maeWindowX = LyricController.windowX;
                            LyricController.maeWindowY = LyricController.windowY;
                            LyricController.maeWindowWidth = LyricController.windowWidth;
                            LyricController.maeWindowHeight = LyricController.windowHeight;

                            LyricController.windowX = -root.bw;
                            LyricController.windowY = -root.bw;
                            LyricController.windowWidth = root.width + 2 * root.bw;
                            LyricController.windowHeight = root.height+ 2 * root.bw;
                        }
                        root.synchronousCoordinates();
                        LyricController.isFullScreen = !LyricController.isFullScreen;
                    }
                }
                // @todo 设置
                // MusicActionButton
                MusicActionButton {
                    visible: root.showControls
                    Layout.preferredWidth: 24
                    Layout.preferredHeight: 24
                    defaultColor: Theme.highlightingColor
                    hoveredColor: Theme.highlightingColor
                    url: "qrc:/icons/close.svg"
                    onClicked: root.reqClose()
                }
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

    function synchronousCoordinates() {
        root.setRectX(LyricController.windowX);
        root.setRectY(LyricController.windowY);
        root.setRectWidth(LyricController.windowWidth);
        root.setRectHeight(LyricController.windowHeight);
        root.updateMask();
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
