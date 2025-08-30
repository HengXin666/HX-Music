import QtQuick
import QtQuick.Controls
import HX.Music

Rectangle {
    id: root
    property real from: 0
    // 应该初始化为 ms(毫秒) 单位, 以确保拖动时候传参是正确的
    property real to: 1_000 // 1s (移动间隔)
    property real val: 0

    property color completedColor: Theme.highlightingColor
    property color remainingColor: "#cf555555"
    property real barHeight: 2
    property real hoverBarHeight: 6
    property real handleRadius: 6
    property int hoverMargin: 5   // 统一悬浮/拖动判定扩展

    width: 300
    height: 200
    color: "transparent"

    // 背景进度条
    Rectangle {
        id: progressBg
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: root.barHeight
        color: root.remainingColor
        radius: root.barHeight / 2
    }

    // 已完成进度条
    Rectangle {
        id: progress
        anchors.top: progressBg.top
        x: 0
        height: root.barHeight
        width: (root.val - root.from) / (root.to - root.from) * root.width
        color: root.completedColor
        radius: root.hoverBarHeight / 2
    }

    // 鼠标操作层（点击 + 悬浮判定）
    MouseArea {
        id: clickHoverArea
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width
        height: root.barHeight + root.hoverMargin * 2
        y: progressBg.y - root.hoverMargin

        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor

        onClicked: mouse => {
            let newVal = root.from + (mouse.x / root.width) * (root.to - root.from);
            root.val = Math.round(newVal);
            MusicController.setPosition(root.val);
        }

        onEntered: {
            progress.height = root.hoverBarHeight;
            progress.y = progressBg.y - (root.hoverBarHeight - root.barHeight) / 2;
            handleCircle.visible = true;
        }

        onExited: {
            progress.height = root.barHeight;
            progress.y = progressBg.y;
            handleCircle.visible = false;
        }
    }

    // 拖动手柄
    Rectangle {
        id: handle
        width: 12
        height: root.hoverBarHeight + root.hoverMargin * 2
        color: "transparent"
        y: progress.y - root.hoverMargin
        x: progress.width - width / 2

        MouseArea {
            id: handleMouse
            anchors.fill: parent
            cursorShape: Qt.SizeHorCursor
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: -handle.width / 2
            drag.maximumX: root.width - handle.width / 2

            onPositionChanged: {
                let newVal = root.from + ((handle.x + handle.width / 2) / root.width) * (root.to - root.from);
                root.val = Math.round(newVal);
            }

            onPressed: handleCircle.visible = true
            onReleased: handleCircle.visible = false
        }
    }

    // 圆形手柄显示
    Rectangle {
        id: handleCircle
        width: root.handleRadius * 2
        height: root.handleRadius * 2
        radius: root.handleRadius
        color: Theme.highlightingColor
        visible: false
        y: progress.y + progress.height / 2 - root.handleRadius
        x: progress.width - root.handleRadius
    }

    // 进度值变化同步 UI
    onValChanged: {
        progress.width = (root.val - root.from) / (root.to - root.from) * root.width;
        handle.x = progress.width - handle.width / 2;
        handleCircle.x = progress.width - root.handleRadius;
        handleCircle.y = progress.y + progress.height / 2 - root.handleRadius;

        if (handleMouse.pressed) {
            MusicController.setPosition(root.val); // 更新播放位置到指定值, 现手动控制进度条位置改变时同步更新播放位置
        }
    }

    Connections {
        id: songConn
        target: null
        // 绑定信号: 更新歌曲
        function onNewSongLoaded(song: MusicInformation) {
            root.to = song.getLengthInMilliseconds();
        }

        // 绑定信号: 播放位置变化
        function onMusicPlayPosChanged(pos: int) {
            root.val = pos;
        }
    }

    Component.onCompleted: {
        songConn.target = SignalBusSingleton;
    }
}
