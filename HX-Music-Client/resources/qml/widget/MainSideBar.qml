pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window

Item {
    id: root
    width: 240
    height: parent.height

    property int currentIndex: 0
    property int currentPlaylistIndex: -1
    signal tabClicked(int index)
    signal playlistClicked(int index)

    // 歌单数据
    property var playlists: [
        {
            name: "我最喜欢的歌",
            count: 128,
            type: "created"
        },
    ]

    // 整个侧边栏使用ScrollView实现滚动
    ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true

        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff // 隐藏横向滚动条

        ColumnLayout {
            width: scrollView.width
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
                        root.currentPlaylistIndex = -1;
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
                        root.currentPlaylistIndex = -1;
                        root.currentIndex = 1;
                        root.tabClicked(1);
                    }
                }

                // 本地下载
                SidebarItem {
                    icon: "qrc:/icons/download.svg"
                    text: "本地下载"
                    isSelected: root.currentIndex === 2
                    onClicked: {
                        root.currentPlaylistIndex = -1;
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
                        onClicked: playlistOperationBar.currentIndex = 0
                    }

                    // 收藏歌单按钮
                    TextButton {
                        text: "收藏歌单"
                        isSelected: playlistOperationBar.currentIndex === 1
                        onClicked: playlistOperationBar.currentIndex = 1
                    }

                    TextButton {
                        text: "+"
                        textSize: 20
                        onClicked: {
                            console.log("添加歌单按钮点击");
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

            // 歌单列表
            Repeater {
                model: root.playlists

                delegate: PlaylistItem {
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    name: modelData.name
                    songCount: modelData.count
                    isSelected: root.currentPlaylistIndex === index
                    visible: (playlistOperationBar.currentIndex === 0 && modelData.type === "created") 
                          || (playlistOperationBar.currentIndex === 1 && modelData.type === "favorite")

                    onClicked: {
                        root.currentIndex = -1;
                        root.currentPlaylistIndex = index;
                        root.playlistClicked(index);
                    }
                }
            }

            // 底部填充空间
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    // 侧边栏项组件
    component SidebarItem: Rectangle {
        id: sidebarItem
        width: 200
        height: 40

        // 属性定义
        property string icon: ""
        property string text: "菜单项"
        property bool isSelected: false
        property color hoverColor: "#1bffffff"
        signal clicked()

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
                source: `image://svgColored/${sidebarItem.icon}?color=${sidebarItem.isSelected 
                    ? Theme.highlightingColor 
                    : Theme.paratextColor}`
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
        signal clicked()

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

    // 歌单项组件
    component PlaylistItem: Item {
        id: playlistItem
        implicitWidth: parent.width
        implicitHeight: visible ? 50 : 0

        property string name: ""
        property int songCount: 0
        property bool isSelected: false
        signal clicked()

        Rectangle {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            color: playlistItem.isSelected ? "#1bffffff" : "transparent"
            radius: 6

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                // 歌单图标
                Rectangle {
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    radius: 4
                    color: "#e0e0e0"

                    Text {
                        anchors.centerIn: parent
                        text: "♪"
                        color: "#757575"
                        font.pixelSize: 16
                    }
                }

                // 歌单信息
                ColumnLayout {
                    spacing: 2
                    Layout.fillWidth: true

                    Text {
                        text: playlistItem.name
                        color: playlistItem.isSelected ? Theme.highlightingColor  : Theme.paratextColor
                        font.pixelSize: 13
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }

                    Text {
                        text: playlistItem.songCount + "首"
                        color: playlistItem.isSelected ? Theme.textColor : Theme.paratextColor
                        font.pixelSize: 11
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: playlistItem.clicked()
            }
        }
    }
}
