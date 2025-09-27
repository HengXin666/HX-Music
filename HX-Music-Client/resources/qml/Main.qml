pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import HX.Music
import "./widget"
import "./widget/internal"
import "./data"
import "./view"
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
        visible = false;
    }

    function onTrayShow() {
        visible = true;
    }

    function onTrayClose() {
        lyricsState.del();
        Qt.callLater(() => Qt.quit());
    }

    function onTrayTogglePause() {
        MusicController.togglePause();
    }

    function onTrayPrev() {
        MusicController.prev();
    }

    function onTrayNext() {
        MusicController.next();
    }

    titleBar: Rectangle {
        // 自绘标题栏, 可为null, 内部需要自定义 height
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
            Image {
                id: logoImg
                Layout.preferredHeight: 50
                Layout.preferredWidth: 180
                source: "qrc:/logo/HXMusic_logo.png"
                verticalAlignment: Qt.AlignVCenter
                fillMode: Image.PreserveAspectFit
            }
            Label {
                text: mainWin.title
                color: Theme.highlightingColor
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }

            // 头像
            Image {
                id: avatarImage
                source: "qrc:/icons/user.svg"
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                fillMode: Image.PreserveAspectFit
                smooth: true
                clip: true
                // 图片圆角
                layer.enabled: true
                layer.smooth: true
                layer.effect: OpacityMask {
                    maskSource: Rectangle {
                        width: avatarImage.width
                        height: avatarImage.height
                        radius: avatarImage.width / 2
                    }
                }
                // 信号
                Connections {
                    target: UserController
                    function onLoginChanged() {
                        // 登录状态变化时, 刷新头像
                        if (UserController.isLoggedIn()) {
                            // 已登录, 刷新头像
                            avatarImage.source = "image://netImagePoll/user/avatar/get?" + Date.now();
                        } else {
                            // 未登录, 使用默认头像
                            avatarImage.source = "qrc:/icons/user.svg";
                        }
                    }
                }
            }

            // 名称
            Label {
                id: nameLabel
                text: UserController.isLoggedIn() ? UserController.getName() : "未登录"
                color: Theme.textColor
                font.pixelSize: 14
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.rightMargin: 10

                // 连接 UserController 的 name 属性变化
                Connections {
                    target: UserController
                    function onLoginChanged() {
                        const name = UserController.isLoggedIn() ? UserController.getName() : "未登录";
                        if (name !== nameLabel.text) {
                            nameLabel.text = name;
                        }
                    }
                }
            }

            // 设置按钮
            MusicActionButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                Layout.rightMargin: 13
                url: "qrc:/icons/setting.svg"
                onClicked: {
                    mainWin.delegateRef.stackViewRef.currentIndex = 4;
                    mainWin.delegateRef.settingStackView.currentIndex = 1;
                }
            }

            // 最小化按钮
            MusicActionButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                url: "qrc:/icons/dropdown.svg"
                onClicked: mainWin.showMinimized()
            }

            // 最大化/还原按钮
            MusicActionButton {
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                url: mainWin.visibility === Window.Maximized ? "qrc:/icons/restore.svg"   // 还原图标
                 : "qrc:/icons/up.svg"  // 最大化图标
                onClicked: mainWin.visibility === Window.Maximized ? mainWin.showNormal() : mainWin.showMaximized()
            }

            // 关闭按钮
            MusicActionButton {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                Layout.rightMargin: 13
                url: "qrc:/icons/close.svg"
                onClicked: mainWin.close()
            }
        }
    }

    delegate: Rectangle {
        id: rootUI
        focus: true
        anchors.fill: parent
        color: "transparent"

        Keys.enabled: true  // 不设置按键使能，获取不了按键事件
        Keys.onPressed: (event) => {
            if (event.key === Qt.Key_MediaPlay) {
                MusicController.togglePause();
            } else if (event.key === Qt.Key_MediaNext) {
                MusicController.next();
            } else if (event.key === Qt.Key_MediaPrevious) {
                MusicController.prev();
            }
        }

        // 暴露 stackView 引用
        property alias stackViewRef: stackView
        property alias settingStackView: settingStackView

        ColumnLayout {
            anchors.fill: parent

            // 多个标签页面
            RowLayout {
                MainSideBar {
                    Layout.preferredWidth: 200
                    Layout.fillHeight: true
                    onTabClicked: index => {
                        // console.log("点击了标签页:", index);
                        stackView.currentIndex = index; // 属性存储当前标签索引
                        if (index == 0) {
                            // 全部音乐
                            allMusicListView.loadMoreRequested();
                        } else if (index == 1) {
                            // @todo 分类

                        } else if (index == 2) {
                            // @todo 上传列表

                        }
                    }

                    onPlayListClicked: function (id) {
                        // console.log("点击了歌单:", id);
                        stackView.currentIndex = 3;
                        PlaylistController.loadPlaylistById(id);
                    }
                }

                StackLayout {
                    id: stackView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Rectangle {
                        // 0
                        color: "transparent"
                        AllMusicListView {
                            id: allMusicListView
                            anchors.fill: parent
                        }
                    }
                    Rectangle {
                        // 1
                        color: "transparent"
                    }
                    Rectangle {
                        // 2
                        color: "transparent"
                        UploadListView {
                            anchors.fill: parent
                        }
                    }
                    Rectangle {
                        // 3
                        color: "transparent"
                        MusicListView {
                            anchors.fill: parent
                        }
                    }
                    Rectangle {
                        // 4
                        color: "transparent"
                        RowLayout {
                            anchors.fill: parent
                            spacing: 10

                            // 侧边导航 (纯文本, 选择后右边会有 | 表示)
                            ColumnLayout {
                                Layout.preferredWidth: 50
                                Layout.alignment: Qt.AlignTop
                                spacing: 10
                                SideNavItem {
                                    id: loginButton
                                    Layout.preferredWidth: 50
                                    text: "登录"
                                    textColor: Theme.textColor
                                    highlightColor: Theme.highlightingColor
                                    checked: settingStackView.currentIndex === 0
                                    onClicked: {
                                        settingStackView.currentIndex = 0;
                                    }
                                }
                                SideNavItem {
                                    id: settingButton
                                    Layout.preferredWidth: 50
                                    text: "设置"
                                    textColor: Theme.textColor
                                    highlightColor: Theme.highlightingColor
                                    checked: settingStackView.currentIndex === 1
                                    onClicked: {
                                        settingStackView.currentIndex = 1;
                                    }
                                }
                                SideNavItem {
                                    id: userButton
                                    Layout.preferredWidth: 50
                                    text: "用户"
                                    textColor: Theme.textColor
                                    highlightColor: Theme.highlightingColor
                                    checked: settingStackView.currentIndex === 2
                                    onClicked: {
                                        settingStackView.currentIndex = 2;
                                    }
                                }
                                SideNavItem {
                                    id: backendButton
                                    Layout.preferredWidth: 50
                                    text: "后端"
                                    textColor: Theme.textColor
                                    highlightColor: Theme.highlightingColor
                                    checked: settingStackView.currentIndex === 3
                                    onClicked: {
                                        settingStackView.currentIndex = 3;
                                    }
                                }
                                SideNavItem {
                                    id: aboutButton
                                    Layout.preferredWidth: 50
                                    text: "关于"
                                    textColor: Theme.textColor
                                    highlightColor: Theme.highlightingColor
                                    checked: settingStackView.currentIndex === 4
                                    onClicked: {
                                        settingStackView.currentIndex = 4;
                                    }
                                }
                            }

                            // 内容区
                            StackLayout {
                                id: settingStackView
                                Layout.fillWidth: true
                                Layout.fillHeight: true

                                LoginView {}
                                SettingView {}
                                UserView {
                                    anchors.fill: parent
                                }
                                BackendView {}
                                Rectangle {
                                    // 关于页面 @todo
                                    color: "transparent"
                                    ColumnLayout {
                                        anchors.fill: parent
                                        spacing: 10
                                        // 上 + 居中
                                        Layout.alignment: Qt.AlignTop | Qt.AlignCenter
                                        Item {
                                            Layout.fillHeight: true
                                            Layout.alignment: Qt.AlignTop
                                        }
                                        Label {
                                            text: "HX.Music"
                                            font.pixelSize: 24
                                            color: Theme.highlightingColor
                                        }
                                        Label {
                                            text: "版本: 3.0.1-Beta"
                                            color: Theme.textColor
                                        }
                                        Label {
                                            text: "作者: Heng_Xin"
                                            color: Theme.textColor
                                        }
                                        Label {
                                            text: 'Github: <a href="https://github.com/HengXin666/HX-Music">https://github.com/HengXin666/HX-Music</a>'
                                            textFormat: Text.RichText
                                            onLinkActivated: function(link) {
                                                Qt.openUrlExternally(link);
                                            }
                                            color: Theme.textColor
                                        }
                                        Item {
                                            Layout.fillHeight: true
                                            Layout.alignment: Qt.AlignBottom
                                        }
                                    }
                                }
                            }
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

        Connections {
            target: SignalBusSingleton
            // 绑定信号: token 过期, 跳转到登录界面
            function onGotoLoginViewSignal() {
                stackView.currentIndex = 4;
                settingStackView.currentIndex = 0;
            }
        }

    }

    // 背景 (双缓冲渲染)
    Rectangle {
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
            const w = Math.max(1, Math.ceil(width * dpr * 1.0));
            const h = Math.max(1, Math.ceil(height * dpr * 1.0));
            return Qt.size(w, h);
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
                    bgCurrent.source = source;
                    bgNext.opacity = 0;
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                }
            }
        }

        function updateBackground(size) {
            bgNext.sourceSize = size;
            bgNext.source = Theme.backgroundImgUrl;
            bgNext.opacity = 1;
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
            bgCurrent.sourceSize = bk.targetSizeForImage();
            bgCurrent.source = Theme.backgroundImgUrl;
        }

        // 监听 Theme.backgroundImgUrl 变化
        Connections {
            target: Theme
            function onBackgroundImgUrlChanged() {
                bk.updateBackground(bk.targetSizeForImage());
            }
        }
    }
}
