pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import "./internal"

Item {
    id: root

    width: 200
    height: parent.height

    property int currentIndex: 0
    property int currentPlaylistIndex: -1
    signal tabClicked(int index);
    signal playListClicked(int id);

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
                        favoritePlayListView.resetIndex();
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
                        favoritePlayListView.resetIndex();
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
                        createdPlayListView.resetIndex();
                        favoritePlayListView.resetIndex();
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

                    // 新建歌单 / 导入歌单
                    MusicActionButton {
                        url: "qrc://icons/add.svg"
                        onClicked: {
                            console.log("添加歌单按钮点击");
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
                visible: playlistOperationBar.currentIndex === 0
                onPlayListClicked: function(id: int) {
                    root.currentIndex = -1;
                    favoritePlayListView.resetIndex();
                    root.playListClicked(id);
                }
            }

            // 歌单列表 (收藏他人)
            PlaylistView {
                id: favoritePlayListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: playlistOperationBar.currentIndex === 1
                onPlayListClicked: function(id: int) {
                    root.currentIndex = -1;
                    createdPlayListView.resetIndex();
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
}