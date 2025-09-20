pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import HX.Music

Item {
    id: root
    focus: true

    MusicListModel {
        id: musicListModel
    }

    // 拖拽文件
    DropArea {
        anchors.fill: parent
        onDropped: drop => {
            for (const urlStr of drop.urls) {
                let path = decodeURIComponent(urlStr).replace("file://", "");
                // @todo 要实现上传
                musicListModel.addFromPath(path);
            }
        }
    }
    property real maxTitleColumnWidth: 0
    property real maxAlbumColumnWidth: 0

    ListView {
        id: listView
        property bool isSwaped: false // 是否进行了交换

        model: musicListModel
        anchors.fill: parent
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
            height: 60
            z: mouseArea.pressed ? 10 : 1

            MouseArea {
                id: mouseArea
                property bool _hovered: false // 是否悬浮
                anchors.fill: parent
                preventStealing: true // 防止信号被窃取

                // 双击是选择
                onDoubleClicked: (mouse) => {
                    if (mouse.button === Qt.LeftButton) {
                        listView.currentIndex = delegateRoot.index;
                        MusicController.setPlaylistId(musicListModel.getPlaylistId());
                        MusicController.playMusic(musicListModel.getUrl(delegateRoot.index));
                        MusicController.listIndex = delegateRoot.index;
                    }
                }

                // 拖拽
                onPositionChanged: (mouse) => {
                    if (!mouseArea.pressed) {
                        if (listView.isSwaped) {
                            listView.isSwaped = false;
                            // 保存歌单
                            musicListModel.savePlaylist();
                        }
                        return;
                    }
                    let from = delegateRoot.index;
                    // 换算为父上控件的坐标系
                    let p = mouseArea.mapToItem(listView, mouse.x, mouse.y);
                    let to = listView.indexAt(p.x, p.y);
                    if (to !== -1 && to !== from) {
                        listView.isSwaped |= musicListModel.swapRow(from, to);
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
                color: {
                    if (delegateRoot.isSelected)
                        return "#2bffffff";
                    else if (mouseArea._hovered)
                        return "#3fffffff";
                    return "transparent";
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 10

                    // 编号图标
                    PlayStatusButton {
                        Layout.preferredWidth: 40
                        Layout.alignment: Qt.AlignCenter
                        text: (delegateRoot.index + 1).toString()
                        isSelected: delegateRoot.isSelected
                        isHovered: mouseArea._hovered
                        path: musicListModel.getUrl(delegateRoot.index)
                        onSwitchThisMusic: {
                            listView.currentIndex = delegateRoot.index;
                        }
                    }

                    // 封面 + 标题 + 歌手(靠左, 自适应宽度)
                    RowLayout {
                        id: titleColumn
                        Layout.alignment: Qt.AlignLeft
                        spacing: 8
                        Layout.preferredWidth: root.maxTitleColumnWidth
                        onWidthChanged: {
                            if (width > root.maxTitleColumnWidth) {
                                root.maxTitleColumnWidth = width;
                            }
                        }

                        // 封面
                        Rectangle {
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50
                            color: "transparent"
                            Image {
                                id: coverImage
                                anchors.fill: parent
                                source: `image://onlineImagePoll/${delegateRoot.model.id}`
                                // 图片圆角
                                layer.enabled: true
                                layer.smooth: true
                                layer.effect: OpacityMask {
                                    maskSource: Rectangle {
                                        width: coverImage.width
                                        height: coverImage.height
                                        radius: 10
                                    }
                                }
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                            }
                        }

                        // 标题 + 歌手
                        ColumnLayout {
                            spacing: 2
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true

                            // 标题
                            Text {
                                text: delegateRoot.model.title
                                color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.textColor
                                font.pixelSize: 16
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            // 歌手
                            Text {
                                id: artistText
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.paratextColor
                                elide: Text.ElideRight

                                // 动态生成文本
                                property string artistString: {
                                    const list = delegateRoot.model.artist
                                    if (!list || list.length === 0) return ""
                                    return list.join("、")
                                }

                                text: artistString
                            }
                        }
                    }

                    // 专辑(靠左, 自适应宽度)
                    Text {
                        id: albumColumn
                        text: delegateRoot.model.album.length > 16
                                ? delegateRoot.model.album.substring(0, 16) + "…"
                                : delegateRoot.model.album
                        font.pixelSize: 14
                        color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.textColor
                        Layout.alignment: Qt.AlignRight
                        Layout.preferredWidth: root.maxAlbumColumnWidth
                        onWidthChanged: {
                            if (width > root.maxAlbumColumnWidth) {
                                root.maxAlbumColumnWidth = width;
                            }
                        }
                    }

                    // 时长(固定宽度, 永远贴最右)
                    Text {
                        text: root.formatDuration(delegateRoot.model.duration)
                        font.pixelSize: 14
                        color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.paratextColor
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 60
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    }
                }
            }
        }

        Component.onCompleted: {
            // 绑定 当前选择项更新信号
            MusicController.listIndexChanged.connect((idx) => {
                listView.currentIndex = idx;
            });
        }
    }

    // 右键菜单 @todo 美化: 支持圆边
    Menu {
        id: menu
        property int index: -1

        MenuItem {
            text: "播放"
            onTriggered: {
                listView.currentIndex = menu.index;
                MusicController.setPlaylistId(musicListModel.getPlaylistId());
                MusicController.playMusic(musicListModel.getUrl(menu.index));
                MusicController.listIndex = menu.index;
            }
        }
        MenuItem {
            text: "爬取歌词"
            onTriggered: {
                LyricController.crawlKaRaOKAssLyrics(musicListModel.getUrl(menu.index))
            }
        }
        MenuItem {
            text: "删除"
            onTriggered: {
                musicListModel.delMusic(menu.index);
            }
        }
    }

    // 时间格式化
    function formatDuration(seconds) {
        let min = Math.floor(seconds / 60);
        let sec = seconds % 60;
        return (min < 10 ? "0" : "") + min + ":" + (sec < 10 ? "0" : "") + sec;
    }
}
