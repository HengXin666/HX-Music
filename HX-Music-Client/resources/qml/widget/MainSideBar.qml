pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import HX.Music
import "./internal"

Item {
    id: root

    width: 200
    height: parent.height

    property int currentIndex: 0
    property int currentPlaylistIndex: -1
    signal tabClicked(int index)
    signal playListClicked(int id)

    Popup {
        id: addPlaylistPopup
        modal: true // 模态
        focus: true // 焦点
        closePolicy: Popup.CloseOnEscape
        // 设置父对象为覆盖层, 这样坐标就是窗口坐标
        parent: Overlay.overlay
        width: 500
        height: 300
        dim: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        // 窗口弹出时候的动画
        enter: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: 500
            }
        }
        // 窗口关闭时候的动画
        exit: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 500
            }
        }

        // 当前视图状态
        property string currentView: "create" // "create" 或 "import"

        background: null

        // 内容区域
        contentItem: Rectangle {
            anchors.fill: parent
            color: "#ea431e3a"
            radius: 5

            // 标题栏
            Rectangle {
                id: header
                width: parent.width
                height: 50
                color: "#ea990099"
                radius: 5

                // 标题
                Text {
                    id: titleText
                    anchors {
                        left: parent.left
                        leftMargin: 20
                        verticalCenter: parent.verticalCenter
                    }
                    text: addPlaylistPopup.currentView === "create" ? "创建歌单" : "导入歌单"
                    color: Theme.textColor
                    font.bold: true
                    font.pixelSize: 18
                }

                // 关闭按钮
                MusicActionButton {
                    anchors {
                        right: parent.right
                        rightMargin: 16
                        verticalCenter: parent.verticalCenter
                    }
                    width: 16
                    height: 16
                    url: "qrc://icons/close.svg"
                    onClicked: addPlaylistPopup.close()
                }
            }

            // 视图切换选项卡
            Row {
                id: tabBar
                anchors {
                    top: header.bottom
                    topMargin: 15
                    horizontalCenter: parent.horizontalCenter
                }
                spacing: 0

                // 创建歌单选项卡
                Rectangle {
                    width: 120
                    height: 40
                    color: addPlaylistPopup.currentView === "create" ? "#1dffffff" : "transparent"
                    radius: 5

                    Text {
                        anchors.centerIn: parent
                        text: "创建歌单"
                        color: addPlaylistPopup.currentView === "create" ? Theme.highlightingColor : Theme.textColor
                        font.pixelSize: 14
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: addPlaylistPopup.currentView = "create"
                    }
                }

                // 导入歌单选项卡
                Rectangle {
                    width: 120
                    height: 40
                    color: addPlaylistPopup.currentView === "import" ? "#1dffffff" : "transparent"
                    radius: 5

                    Text {
                        anchors.centerIn: parent
                        text: "导入歌单"
                        color: addPlaylistPopup.currentView === "import" ? Theme.highlightingColor : Theme.textColor
                        font.pixelSize: 14
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: addPlaylistPopup.currentView = "import"
                    }
                }
            }

            // 内容区域
            StackLayout {
                id: contentStack
                anchors {
                    top: tabBar.bottom
                    topMargin: 20
                    left: parent.left
                    right: parent.right
                    bottom: buttonRow.top
                    bottomMargin: 20
                    leftMargin: 20
                    rightMargin: 20
                }
                currentIndex: addPlaylistPopup.currentView === "create" ? 0 : 1

                // 创建歌单视图
                ColumnLayout {
                    spacing: 15

                    Text {
                        text: "歌单名称"
                        font.pixelSize: 14
                        color: Theme.textColor
                    }

                    TextField {
                        id: playlistNameField
                        Layout.fillWidth: true
                        placeholderText: "请输入歌单名称"
                        background: Canvas {
                            implicitHeight: 40

                            onPaint: {
                                const ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                ctx.strokeStyle = Theme.paratextColor;
                                ctx.lineWidth = 1;

                                // 绘制下边框
                                ctx.beginPath();
                                ctx.moveTo(0, height - 0.5);
                                ctx.lineTo(width, height - 0.5);
                                ctx.stroke();

                                // 绘制左边框下半部分
                                ctx.beginPath();
                                ctx.moveTo(0.5, height / 2);
                                ctx.lineTo(0.5, height);
                                ctx.stroke();

                                // 绘制右边框下半部分
                                ctx.beginPath();
                                ctx.moveTo(width - 0.5, height / 2);
                                ctx.lineTo(width - 0.5, height);
                                ctx.stroke();
                            }
                        }
                        placeholderTextColor: Theme.paratextColor
                        color: Theme.textColor
                        font {
                            pixelSize: 18
                            family: "Microsoft YaHei"
                        }
                    }

                    Text {
                        text: "歌单名称长度建议在2-20个字符之间"
                        font.pixelSize: 12
                        color: Theme.paratextColor
                    }
                }

                // 导入歌单视图
                ColumnLayout {
                    spacing: 15

                    Text {
                        text: "歌单URL"
                        font.pixelSize: 14
                        color: Theme.textColor
                    }

                    TextField {
                        id: playlistUrlField
                        Layout.fillWidth: true
                        placeholderText: "请输入歌单分享链接"
                        background: Canvas {
                            implicitHeight: 40

                            onPaint: {
                                const ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                ctx.strokeStyle = Theme.paratextColor;
                                ctx.lineWidth = 1;

                                // 绘制下边框
                                ctx.beginPath();
                                ctx.moveTo(0, height - 0.5);
                                ctx.lineTo(width, height - 0.5);
                                ctx.stroke();

                                // 绘制左边框下半部分
                                ctx.beginPath();
                                ctx.moveTo(0.5, height / 2);
                                ctx.lineTo(0.5, height);
                                ctx.stroke();

                                // 绘制右边框下半部分
                                ctx.beginPath();
                                ctx.moveTo(width - 0.5, height / 2);
                                ctx.lineTo(width - 0.5, height);
                                ctx.stroke();
                            }
                        }
                        placeholderTextColor: Theme.paratextColor
                        color: Theme.textColor
                        font {
                            pixelSize: 18
                            family: "Microsoft YaHei"
                        }
                    }

                    Text {
                        // @todo
                        text: "支持从酷狗音乐、网易云、QQ音乐等平台导入公开歌单"
                        font.pixelSize: 12
                        color: Theme.paratextColor
                        wrapMode: Text.Wrap
                    }
                }
            }

            // 底部按钮区域
            RowLayout {
                id: buttonRow
                anchors {
                    bottom: parent.bottom
                    bottomMargin: 20
                    left: parent.left
                    right: parent.right
                    leftMargin: 20
                    rightMargin: 20
                }
                spacing: 15

                // 左侧占位空间
                Item {
                    Layout.fillWidth: true
                }

                // 取消按钮
                Button {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 50

                    contentItem: Text {
                        text: "取消"
                        color: Theme.textColor
                        font {
                            pixelSize: 16
                            family: "Microsoft YaHei" // 使用更美观的字体
                        }
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        radius: 6
                        color: parent.hovered ? "#3a3a3a" : "#2b2b2b"
                        border {
                            width: 1
                            color: "#555555"
                        }
                    }

                    onClicked: addPlaylistPopup.close()
                }

                // 创建/导入按钮
                Button {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 50

                    contentItem: Text {
                        text: addPlaylistPopup.currentView === "create" ? "创建" : "导入"
                        color: Theme.textColor
                        font {
                            pixelSize: 16
                            family: "Microsoft YaHei"
                            bold: true // 主要操作按钮加粗
                        }
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    background: Rectangle {
                        radius: 6
                        color: parent.hovered ? "#4CAF50" : "#388E3C" // 绿色主题，悬停时变亮
                    }

                    onClicked: {
                        if (addPlaylistPopup.currentView === "create") {
                            console.log("创建歌单:", playlistNameField.text);
                            // 这里添加创建歌单的逻辑
                            PlaylistController.makePlaylist(playlistNameField.text, "");
                        } else {
                            console.log("导入歌单:", playlistUrlField.text);
                            // 这里添加导入歌单的逻辑
                        }
                        addPlaylistPopup.close();
                    }
                }
            }
        }
    }

    // 整个侧边栏使用ScrollView实现滚动
    ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
            id: contentColumn
            width: root.width
            Layout.preferredWidth: width
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0

            // 顶部功能区域
            ColumnLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.topMargin: 10
                Layout.leftMargin: 12
                Layout.rightMargin: 12

                // 音乐
                SidebarItem {
                    icon: "qrc:/icons/home.svg"
                    text: "音乐"
                    isSelected: root.currentIndex === 0
                    onClicked: {
                        createdPlayListView.resetIndex();
                        root.currentIndex = 0;
                        root.tabClicked(0);
                    }
                }

                // 我的收藏
                SidebarItem {
                    icon: "qrc:/icons/follow_gray.svg"
                    text: "我的收藏"
                    isSelected: root.currentIndex === 1
                    onClicked: {
                        createdPlayListView.resetIndex();
                        root.currentIndex = 1;
                        root.tabClicked(1);
                        // @todo 到时候跳转到 '我喜欢'
                    }
                }

                // 本地下载
                SidebarItem {
                    icon: "qrc:/icons/upload.svg"
                    text: "上传列表"
                    isSelected: root.currentIndex === 2
                    onClicked: {
                        createdPlayListView.resetIndex();
                        root.currentIndex = 2;
                        root.tabClicked(2);
                    }
                }
            }

            // 歌单操作栏
            Item {
                id: playlistOperationBar
                Layout.fillWidth: true
                Layout.topMargin: 20
                Layout.bottomMargin: 10
                implicitHeight: 40

                // 歌单选项
                property int currentIndex: 0

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 16

                    // 自建歌单按钮
                    TextButton {
                        text: "自建歌单"
                        isSelected: playlistOperationBar.currentIndex === 0
                        onClicked: {
                            playlistOperationBar.currentIndex = 0;
                            // @todo 请求: 获取用户创建歌单
                        }
                    }

                    // 收藏歌单按钮
                    TextButton {
                        text: "收藏歌单"
                        isSelected: playlistOperationBar.currentIndex === 1
                        onClicked: {
                            playlistOperationBar.currentIndex = 1;
                            // @todo 请求: 获取用户收藏歌单
                        }
                    }

                    // 新建歌单 / 导入歌单
                    MusicActionButton {
                        url: "qrc://icons/add.svg"
                        onClicked: {
                            console.log("添加歌单按钮点击");
                            addPlaylistPopup.open();
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

            // 歌单列表 (个人创建)
            PlaylistView {
                id: createdPlayListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                onPlayListClicked: function (id) {
                    root.currentIndex = -1;
                    root.playListClicked(id);
                }
            }
        }
    }

    // 侧边栏项组件
    component SidebarItem: Rectangle {
        id: sidebarItem
        width: 200 - 12 * 3
        height: 40

        // 属性定义
        property string icon: ""
        property string text: "菜单项"
        property bool isSelected: false
        property color hoverColor: "#1bffffff"
        signal clicked

        // 背景颜色根据状态变化
        color: isSelected || mouseArea.containsMouse ? hoverColor : "transparent"

        // 鼠标区域
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                sidebarItem.clicked();
            }
        }

        // 左侧图标
        Rectangle {
            id: iconContainer
            width: 24
            height: 24
            anchors {
                left: parent.left
                leftMargin: 16
                verticalCenter: parent.verticalCenter
            }
            color: "transparent"

            Image {
                id: iconImage
                anchors.centerIn: parent
                source: `image://svgColored/${sidebarItem.icon}?color=${sidebarItem.isSelected ? Theme.highlightingColor : Theme.paratextColor}`
                width: 20
                height: 20
                fillMode: Image.PreserveAspectFit
            }
        }

        // 右侧文本
        Text {
            id: itemText
            text: sidebarItem.text
            anchors {
                left: iconContainer.right
                leftMargin: 16
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 10
            }
            color: Theme.paratextColor
            font.pixelSize: 14
            font.family: "Microsoft YaHei"
            elide: Text.ElideRight
        }

        // 添加动画效果
        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
    }

    // 文本按钮组件
    component TextButton: Item {
        id: textBtn
        implicitWidth: buttonText.implicitWidth
        implicitHeight: 28

        property string text: ""
        property bool isSelected: false
        property alias textSize: buttonText.font.pixelSize
        signal clicked

        Text {
            id: buttonText
            anchors.centerIn: parent
            text: textBtn.text
            color: textBtn.isSelected ? Theme.textColor : Theme.paratextColor
            font.pixelSize: 12
            font.bold: textBtn.isSelected
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: textBtn.clicked()
        }
    }
}
