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
                musicListModel.addFromPath(path);
            }
        }
    }

    property real maxTitleColumnWidth: 0
    property real maxAlbumColumnWidth: 0

    // 拖拽相关属性
    property int dragSourceIndex: -1
    property int dragTargetIndex: -1
    property bool isDragging: false
    property real dragY: 0

    // 拖拽指示器
    Rectangle {
        id: dragIndicator
        visible: isDragging
        width: listView.width
        height: 2
        color: Theme.highlightingColor
        x: 0
        y: dragY
        z: 100
    }

    ListView {
        id: listView
        model: musicListModel
        anchors.fill: parent
        clip: true
        interactive: true

        // 当前项动画
        move: Transition {
            NumberAnimation { property: "y"; duration: 200; easing.type: Easing.OutQuad }
        }

        // 被挤开的项动画
        moveDisplaced: Transition {
            NumberAnimation { property: "y"; duration: 200; easing.type: Easing.OutQuad }
        }

        // 添加滚动区域识别, 防止拖拽时误触发滚动
        boundsBehavior: Flickable.StopAtBounds

        delegate: Item {
            id: delegateRoot
            required property int index
            required property var model

            property bool isSelected: listView.currentIndex == delegateRoot.index
            property bool isDragTarget: root.dragTargetIndex === delegateRoot.index

            width: listView.width
            height: 60
            z: dragArea.drag.active ? 10 : (isSelected ? 2 : 1)

            // 使用DragHandler替代MouseArea来处理拖拽
            DragHandler {
                id: dragHandler
                target: null // 我们不移动实际的item, 只是处理手势
                acceptedDevices: PointerDevice.Mouse | PointerDevice.Stylus
                grabPermissions: PointerHandler.CanTakeOverFromAnything

                onActiveChanged: {
                    if (active) {
                        // 开始拖拽
                        root.startDrag(delegateRoot.index, mapToItem(listView, width / 2, height / 2).y);
                    } else {
                        // 结束拖拽
                        root.endDrag();
                    }
                }
            }

            // 单独的MouseArea处理点击和悬停
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                preventStealing: false // 允许DragHandler接管
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onEntered: _hovered = true;
                onExited: _hovered = false;
                property bool _hovered: false

                onDoubleClicked: (mouse) => {
                    if (mouse.button === Qt.LeftButton) {
                        listView.currentIndex = delegateRoot.index;
                        MusicController.setPlaylistId(musicListModel.getPlaylistId());
                        MusicController.playMusic(musicListModel.getUrl(delegateRoot.index));
                        MusicController.listIndex = delegateRoot.index;
                    }
                }

                onPressed: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        menu.index = delegateRoot.index;
                        let p = mapToItem(root, mouse.x, mouse.y);
                        menu.x = p.x;
                        menu.y = p.y;
                        menu.visible = true;
                    }
                }
            }

            // 拖拽区域 - 只在特定区域可拖拽 (比如左侧拖拽手柄)
            MouseArea {
                id: dragArea
                width: 40
                height: parent.height
                anchors.left: parent.left
                drag.target: dragItem
                drag.axis: Drag.YAxis
                cursorShape: Qt.SizeVerCursor

                property real startY: 0

                onPressed: (mouse) => {
                    startY = mapToItem(listView, 0, mouse.y).y;
                    root.startDrag(delegateRoot.index, startY);
                }

                onPositionChanged: (mouse) => {
                    if (pressed) {
                        let currentY = mapToItem(listView, 0, mouse.y).y;
                        root.updateDragPosition(currentY);
                    }
                }

                onReleased: {
                    root.endDrag();
                }

                onCanceled: {
                    root.endDrag();
                }

                Rectangle {
                    id: dragItem
                    width: parent.width
                    height: parent.height
                    x: 0
                    y: 0
                    visible: false // 只是用于拖拽计算, 不显示
                }
            }

            Rectangle {
                anchors.fill: parent
                color: {
                    if (delegateRoot.isSelected)
                        return "#2bffffff";
                    else if (mouseArea._hovered)
                        return "#3fffffff";
                    else if (delegateRoot.isDragTarget)
                        return "#1affffff";
                    return "transparent";
                }

                // 拖拽手柄图标
                Rectangle {
                    width: 20
                    height: parent.height
                    color: "transparent"

                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Repeater {
                            model: 3
                            Rectangle {
                                width: 12
                                height: 2
                                radius: 1
                                color: delegateRoot.isSelected ? Theme.highlightingColor : "#666"
                            }
                        }
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 40 // 为拖拽手柄留出空间
                    anchors.margins: 5
                    spacing: 10

                    // 编号图标
                    PlayStatusButton {
                        Layout.preferredWidth: 40
                        Layout.leftMargin: -10
                        Layout.alignment: Qt.AlignCenter
                        text: (delegateRoot.index + 1).toString()
                        isSelected: delegateRoot.isSelected
                        isHovered: mouseArea._hovered
                        path: musicListModel.getUrl(delegateRoot.index)
                        onSwitchThisMusic: {
                            listView.currentIndex = delegateRoot.index;
                        }
                    }

                    // 封面 + 标题 + 歌手
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

                            Text {
                                text: delegateRoot.model.title
                                color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.textColor
                                font.pixelSize: 16
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Text {
                                id: artistText
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                color: delegateRoot.isSelected ? Theme.highlightingColor : Theme.paratextColor
                                elide: Text.ElideRight

                                property string artistString: {
                                    const list = delegateRoot.model.artist;
                                    if (!list || list.length === 0)
                                        return "";
                                    return list.join("、");
                                }

                                text: artistString
                            }
                        }
                    }

                    // 专辑
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

                    // 时长
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
            MusicController.listIndexChanged.connect((idx) => {
                listView.currentIndex = idx;
            });
        }
    }

    // 开始拖拽
    function startDrag(sourceIndex, yPos) {
        dragSourceIndex = sourceIndex;
        isDragging = true;
        dragY = yPos;
        dragTargetIndex = -1;
    }

    // 更新拖拽位置
    function updateDragPosition(yPos) {
        if (!isDragging) return;

        dragY = yPos;

        // 考虑ListView的contentY偏移
        let adjustedY = yPos + listView.contentY;
        let targetIndex = listView.indexAt(10, adjustedY);

        if (targetIndex !== -1 && targetIndex !== dragTargetIndex) {
            dragTargetIndex = targetIndex;

            // 立即交换(提供即时反馈)
            if (dragTargetIndex !== dragSourceIndex) {
                musicListModel.swapRow(dragSourceIndex, dragTargetIndex);
                dragSourceIndex = dragTargetIndex; // 更新源索引
            }
        }

        // 自动滚动当拖拽到边缘时
        autoScroll(yPos);
    }

    // 自动滚动功能
    function autoScroll(yPos) {
        const scrollMargin = 50; // 距离边缘多少像素开始滚动
        const scrollSpeed = 10; // 滚动速度

        if (yPos < scrollMargin && listView.contentY > 0) {
            // 向上滚动
            listView.contentY = Math.max(0, listView.contentY - scrollSpeed);
        } else if (yPos > listView.height - scrollMargin &&
                  listView.contentY < listView.contentHeight - listView.height) {
            // 向下滚动
            listView.contentY = Math.min(listView.contentHeight - listView.height,
                                        listView.contentY + scrollSpeed);
        }
    }

    // 结束拖拽
    function endDrag() {
        if (isDragging) {
            isDragging = false;
            dragTargetIndex = -1;

            // 保存歌单
            if (dragSourceIndex !== -1) {
                musicListModel.savePlaylist();
            }
        }
    }

    // 右键菜单
    Menu {
        id: menu
        property int index: -1
        property var dynamicModel: []

        onAboutToShow: {
            dynamicModel = PlaylistController.getPlaylists();
        }

        MenuItem {
            text: "播放"
            onTriggered: {
                listView.currentIndex = menu.index;
                MusicController.setPlaylistId(musicListModel.getPlaylistId());
                MusicController.playMusic(musicListModel.getUrl(menu.index));
                MusicController.listIndex = menu.index;
            }
        }
        Menu {
            title: "添加到歌单"
            Repeater {
                model: menu.dynamicModel
                delegate: MenuItem {
                    required property var modelData
                    text: modelData.name
                    onTriggered: {
                        const musicId = musicListModel.getUrl(menu.index);
                        if (musicId) {
                            PlaylistController.addMusicToPlaylist(
                                modelData.id, musicId
                            );
                        }
                    }
                }
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

    function formatDuration(seconds) {
        let min = Math.floor(seconds / 60);
        let sec = seconds % 60;
        return (min < 10 ? "0" : "") + min + ":" + (sec < 10 ? "0" : "") + sec;
    }
}