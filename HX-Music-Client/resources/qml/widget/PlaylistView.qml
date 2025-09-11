pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import HX.Music

Item {
    id: root
    focus: true

    PlaylistModel {
        id: playlistModel
    }

    signal playListClicked(int id);

    function resetIndex() {
        listView.currentIndex = -1;
    }

    implicitHeight: listView.contentHeight

    ListView {
        id: listView
        property bool isSwaped: false // 是否进行了交换

        model: playlistModel
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 5   // 元素之间的上下间距
        clip: true
        interactive: true
        // snapMode: ListView.SnapToItem   // 松手自动对齐
        // boundsBehavior: Flickable.DragOverBounds // 允许拖出时自动滚动

        // 当前项动画
        move: Transition {
            NumberAnimation { property: "y"; duration: 200; easing.type: Easing.OutQuad }
        }

        // 被挤开的项动画
        moveDisplaced: Transition {
            NumberAnimation { property: "y"; duration: 200; easing.type: Easing.OutQuad }
        }

        delegate: Item {
            id: delegateRoot
            required property int index
            required property var model

            // 是否选中
            property bool isSelected: listView.currentIndex == delegateRoot.index
            
            width: listView.width
            height: 48
            z: mouseArea.pressed ? 10 : 1

            MouseArea {
                id: mouseArea
                property bool _hovered: false // 是否悬浮
                anchors.fill: parent
                preventStealing: true // 防止信号被窃取

                // 选择
                onClicked: (mouse) => {
                    if (mouse.button === Qt.LeftButton) {
                        listView.currentIndex = delegateRoot.index;
                        root.playListClicked(playlistModel.getId(listView.currentIndex));
                    }
                }

                // 拖拽
                onPositionChanged: (mouse) => {
                    if (!mouseArea.pressed) {
                        if (listView.isSwaped) {
                            listView.isSwaped = false;
                            // 保存歌单
                            // console.log("我保存了", Date.now()); // @debug
                            // playListModel.savePlaylist();
                        }
                        return;
                    }
                    let from = delegateRoot.index;
                    // 换算为父上控件的坐标系
                    let p = mouseArea.mapToItem(listView, mouse.x, mouse.y);
                    let to = listView.indexAt(p.x, p.y);
                    if (to !== -1 && to !== from) {
                        listView.isSwaped |= playlistModel.swapRow(from, to);
                    }
                }

                hoverEnabled: true
                onEntered: _hovered = true;
                onExited: _hovered = false;

                acceptedButtons: Qt.LeftButton | Qt.RightButton
                // 右键开菜单
                onPressed: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        // 设置菜单的当前索引（供菜单项使用）
                        menu.index = delegateRoot.index;

                        // 换算为父上控件的坐标系
                        let p = mouseArea.mapToItem(root, mouse.x, mouse.y);
                        menu.x = p.x;
                        menu.y = p.y;
                        menu.visible = true;
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: delegateRoot.isSelected || mouseArea._hovered ? "#1bffffff" : "transparent"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 10

                    // 封面 + 歌单名 + 歌曲计数
                    RowLayout {
                        id: titleColumn
                        Layout.alignment: Qt.AlignLeft
                        spacing: 8

                        // @todo 专门的从网络加载的内存的图片缓存...
                        // 封面
                        // Rectangle {
                        //     Layout.preferredWidth: 50
                        //     Layout.preferredHeight: 50
                        //     color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.textColor
                        //     Image {
                        //         anchors.fill: parent
                        //         source: `image://onlineImagePoll/${delegateRoot.model.id}`
                        //         fillMode: Image.PreserveAspectCrop
                        //         asynchronous: true
                        //     }
                        // }

                        // 歌单图标
                        Rectangle {
                            Layout.preferredWidth: 36
                            Layout.preferredHeight: 36
                            Layout.leftMargin: 8
                            radius: 4
                            color: "#e0e0e0"

                            Text {
                                anchors.centerIn: parent
                                text: "♪"
                                color: "#757575"
                                font.pixelSize: 16
                            }
                        }

                        // 歌单简介 [名称 + 计数]
                        ColumnLayout {
                            spacing: 2
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true

                            // 歌单名称
                            Text {
                                text: delegateRoot.model.name
                                color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.paratextColor
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            // 歌单歌曲数量
                            Text {
                                id: artistText
                                font.pixelSize: 12
                                Layout.fillWidth: true
                                color: delegateRoot.isSelected ? Theme.textColor : Theme.paratextColor
                                elide: Text.ElideRight
                                text: `${delegateRoot.model.cnt} 首`
                            }
                        }
                    }
                }
            }
        }

        function init() {
            root.resetIndex();
        }

        Component.onCompleted: {
            Qt.callLater(() => {
                init();
            });;
        }
    }

    // 右键菜单 @todo 美化: 支持圆边
    Menu {
        id: menu
        property int index: -1

        MenuItem {
            text: "播放"
            onTriggered: {
                // @todo
            }
        }
        MenuItem {
            text: "刷新"
            onTriggered: {
                console.log("刷新");
                PlaylistController.refreshPlaylist();
            }
        }
        MenuItem {
            text: "删除"
            onTriggered: {
                PlaylistController.delPlaylist(playlistModel.getId(menu.index));
            }
        }
    }
}
