pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import HX.Music

// 全部音乐列表视图
// 显示为自适应网格布局, 显示全部音乐
// 内容为大图片(封面)(上边)(小圆角) + 下边显示标题 + 艺术家

Item {
    id: root
    width: 800
    height: 600

    AllMusicListModel {
        id: musicModel
    }

    // 布局参数
    property int minItemWidth: 160
    property int idealItemWidth: 200
    property int itemSpacing: 12
    property int loadThreshold: 200    // 距底部多少像素触发加载
    property int loadDebounceMs: 1200  // 防抖时间
    property bool autoLoad: true

    // 计算列数
    property int columnsCount: Math.max(1, Math.floor((width - itemSpacing) / (idealItemWidth + itemSpacing)))

    // 状态与信号
    property bool loading: false
    signal itemClicked(int index, string title, string artist) // @todo 更加深入的介绍

    // 加载更多
    function loadMoreRequested() {
        musicModel.loadMoreRequested();
    }

    GridView {
        id: gridView
        anchors.fill: parent
        anchors.margins: 0
        model: musicModel
        interactive: true
        clip: true
        cacheBuffer: height * 2  // 增加缓存区域提高滚动性能

        cellWidth: Math.max(root.minItemWidth, (width - (root.columnsCount - 1) * root.itemSpacing) / root.columnsCount)
        cellHeight: cellWidth + 60

        delegate: Item {
            id: itemRoot
            width: gridView.cellWidth
            height: gridView.cellHeight

            required property int index
            required property var model

            // 背景高亮效果
            Rectangle {
                id: background
                anchors.fill: parent
                anchors.margins: 4
                radius: 12
                color: mouseArea.containsMouse ? "#22ffffff" : "transparent"

                transitions: Transition {
                    ColorAnimation { duration: 200 }
                }
            }

            Column {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // 封面图片容器
                Item {
                    width: parent.width
                    height: width

                    // 封面图片
                    Image {
                        id: coverImage
                        width: parent.width
                        height: width
                        // 裁剪为 正方形, 全部在图片内部, 保持比例
                        fillMode: Image.PreserveAspectCrop
                        source: `image://netImagePoll/cover/select/${itemRoot.model.id}`
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
                        cache: true
                        asynchronous: true
                        mipmap: true  // 使用mipmap提高缩放质量
                    }

                    // 播放按钮(悬浮时显示)
                    Rectangle {
                        id: playButton
                        width: 48
                        height: 48
                        radius: 24
                        color: "#AA000000"
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.margins: 6
                        opacity: mouseArea.containsMouse ? 1 : 0
                        visible: opacity > 0

                        Behavior on opacity {
                            NumberAnimation { duration: 200 }
                        }

                        Image {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            source: `image://svgColored/qrc:/icons/play.svg?color=${Theme.highlightingColor}`
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                MusicController.playMusic(musicModel.getUrl(itemRoot.index));
                                musicModel.setPlayQueue(itemRoot.index);
                            }
                        }
                    }
                }

                // 标题
                Text {
                    id: titleText
                    width: parent.width
                    text: itemRoot.model.title
                    font.pixelSize: 14
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    color: Theme.textColor
                }

                // 艺术家 @todo 支持点击
                Text {
                    id: artistText
                    // 动态生成文本
                    property string artistString: {
                        const list = itemRoot.model.artist;
                        if (!list || list.length === 0)
                            return "";
                        return list.join("、");
                    }

                    text: artistString
                    width: parent.width
                    font.pixelSize: 12
                    color: Theme.paratextColor
                    elide: Text.ElideRight
                    maximumLineCount: 1
                }
            }

            // 点击区域
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: root.itemClicked(itemRoot.index, itemRoot.model.title, itemRoot.model.artist)

                // 右键菜单
                acceptedButtons: Qt.RightButton
                onPressed: (mouse) => {
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.index = itemRoot.index;
                        contextMenu.modelData = itemRoot.model;
                        contextMenu.popup();
                    }
                }
            }

            // 右键菜单
            Menu {
                id: contextMenu
                property int index: -1
                property var modelData: null

                property var dynamicModel: [] // 用于绑定 Repeater 的临时数据

                onAboutToShow: {
                    dynamicModel = PlaylistController.getPlaylists(); // 每次重新获取
                }

                MenuItem {
                    text: "播放"
                    onTriggered: {
                        if (contextMenu.index >= 0) {
                            MusicController.playMusic(musicModel.getUrl(contextMenu.index))
                            musicModel.setPlayQueue(contextMenu.index);
                        }
                    }
                }

                Menu {
                    title: "添加到歌单"
                    Repeater {
                        model: contextMenu.dynamicModel
                        delegate: MenuItem {
                            required property var modelData
                            text: modelData.name
                            onTriggered: {
                                if (contextMenu.modelData) {
                                    PlaylistController.addMusicToPlaylist(
                                        modelData.id, contextMenu.modelData.id
                                    );
                                }
                            }
                        }
                    }
                }

                MenuItem {
                    text: "详细信息"
                    onTriggered: {
                        if (contextMenu.modelData) {
                            // 这里需要显示详细信息
                            console.log("显示详细信息:", contextMenu.modelData.title)
                            // 例如: showMusicDetailDialog(contextMenu.modelData)
                        }
                    }
                }
            }
        }

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        onContentYChanged: {
            if (!root.autoLoad || root.loading)
                return;
            if (contentHeight <= height)
                return;

            // 检查是否接近底部
            if (contentY + height >= contentHeight - root.loadThreshold) {
                root.tryLoadMore();
            }
        }
    }

    // 防抖计时器
    Timer {
        id: loadTimer
        interval: root.loadDebounceMs
        repeat: false
        onTriggered: root.loading = false
    }

    // 暴露的加载方法
    function tryLoadMore() {
        if (loading)
            return;

        loading = true;
        loadMoreRequested();  // 发出信号, 让外部处理加载

        // 启动防抖计时器
        loadTimer.restart();
    }

    // 手动强制加载
    function forceLoadMore() {
        if (loading)
            return;

        loading = true;
        loadMoreRequested();  // 发出信号, 让外部处理加载

        // 启动防抖计时器
        loadTimer.restart();
    }

    // 加载中指示器
    BusyIndicator {
        id: busyIndicator
        running: root.loading
        visible: root.loading
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        width: 40
        height: 40
    }

    // 当宽度变化时刷新布局
    onWidthChanged: Qt.callLater(gridView.forceLayout)

    // 重置加载状态
    function resetLoading() {
        loading = false;
        loadTimer.stop();
    }
}