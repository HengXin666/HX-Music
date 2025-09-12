pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music
import "./widget"
import "./data"
import "./window"

BorderlessWindow {
    id: mainWin
    minimumWidth: 1080
    minimumHeight: 720
    visible: true
    title: "HX.Music"
    showBorder: false

    // === 全局状态 ===
    property var lyricsState: LyricsState {} // 歌词悬浮窗口状态控制

    onClosing: {
        lyricsState.del();
        Qt.quit(); // 强制退出应用
    }

    titleBar: Rectangle { // 自绘标题栏, 可为null, 内部需要自定义 height
        height: 50
        color: "transparent"
        MouseArea {
            anchors.fill: parent
            onDoubleClicked: mainWin.toggleMaximized()
            acceptedButtons: Qt.LeftButton
            // 不要拖动, 移动逻辑交给外面的 DragHandler
        }
        RowLayout {
            anchors.fill: parent
            Label {
                text: mainWin.title
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            // 最小化按钮
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Image {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "qrc:/icons/dropdown.svg"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: mainWin.showMinimized()
                }
            }

            // 最大化/还原按钮
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Image {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: mainWin.visibility === Window.Maximized
                            ? "qrc:/icons/restore.svg"   // 还原图标
                            : "qrc:/icons/up.svg"  // 最大化图标
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (mainWin.visibility === Window.Maximized)
                            mainWin.showNormal();
                        else
                            mainWin.showMaximized();
                    }
                }
            }

            // 关闭按钮
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Image {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "qrc:/icons/close.svg"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: mainWin.close()
                }
            }
        }
    }

    delegate: Rectangle {
        id: rootUI
        focus: true
        anchors.fill: parent
        color: "transparent"

        ColumnLayout {
            anchors.fill: parent

            // 多个标签页面
            RowLayout {
                MainSideBar {
                    Layout.preferredWidth: 200
                    Layout.fillHeight: true
                    onTabClicked: (index) => {
                        console.log("点击了标签页:", index);
                        stackView.currentIndex = index; // 属性存储当前标签索引
                        if (index == 1) {
                            // @todo 我的收藏
                        } else if (index == 2) {
                            // @todo 上传列表
                        }
                    }

                    onPlayListClicked: function (id: int) {
                        console.log("点击了歌单:", id);
                        stackView.currentIndex = 3;
                        PlaylistController.loadPlaylistById(id);
                    }
                }

                StackLayout {
                    id: stackView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle { 
                        color: "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "HX-Music"
                            color: Theme.highlightingColor 
                            font.pixelSize: 20
                        }
                    }
                    Rectangle {
                        color: "#25d0eaff"                        
                        // 测试按钮
                        Button {
                            text: "测试消息"
                            onClicked: {
                                MessageController.showInfo("这是一条信息消息")
                                MessageController.showError("这是一条错误消息")
                                MessageController.showWarning("这是一条警告消息")
                                MessageController.showSuccess("这是一条成功消息")
                            }
                        }
                    }
                    Rectangle { color: "#7e0b3f6a" }
                    Rectangle {
                        color: "transparent"
                        MusicListView {
                            anchors.fill: parent
                        }
                    }
                }
            }

            // 音乐播放操作条
            PlaybackBar {
                itemHeight: 72
                Layout.fillWidth: true
            }
        }

        // 消息管理器
        MessageManager {
            z: 9999
            focus: true
            anchors.fill: parent
        }
    }

    // 背景 (双缓冲渲染)
    Item {
        id: bk
        x: mainWin.bw
        y: mainWin.bw
        width: mainWin.width - 2 * mainWin.bw
        height: mainWin.height - 2 * mainWin.bw
        z: -10

        // 当前窗口的 DPR
        readonly property real dpr: Screen.devicePixelRatio

        // 计算符合当前窗口的最佳解码分辨率(避免过大或过小)
        function targetSizeForImage(): size {
            // 可按需给点超采样裕量, 例如 ×1.2, 抗缩放抖动
            const w = Math.max(1, Math.ceil(width  * dpr * 1.0))
            const h = Math.max(1, Math.ceil(height * dpr * 1.0))
            return Qt.size(w, h)
        }

        Image {
            id: bgCurrent
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            cache: true
            mipmap: true
            smooth: true
        }

        Image {
            id: bgNext
            anchors.fill: parent
            fillMode: Image.PreserveAspectCrop
            cache: true
            mipmap: true
            smooth: true
            opacity: 0
            onStatusChanged: {
                if (status === Image.Ready) {
                    bgCurrent.source = source
                    bgNext.opacity = 0
                }
            }
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        function updateBackground(size) {
            bgNext.sourceSize = size
            bgNext.source = Theme.backgroundImgUrl
            bgNext.opacity = 1
        }

        Timer {
            id: resizeThrottle
            interval: 120
            repeat: false
            onTriggered: bk.updateBackground(bk.targetSizeForImage())
        }

        onWidthChanged: resizeThrottle.restart()
        onHeightChanged: resizeThrottle.restart()
        Component.onCompleted: {
            bgCurrent.sourceSize = bk.targetSizeForImage()
            bgCurrent.source = Theme.backgroundImgUrl
        }
    }
}