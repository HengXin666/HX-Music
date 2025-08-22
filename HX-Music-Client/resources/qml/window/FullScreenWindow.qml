pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window 2.3
import QtQuick.Controls
import HX.Music

Window {
    id: root
    visible: true
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.BypassWindowManagerHint
    color: "transparent"
    width: Screen.width
    height: Screen.height

    property string windowTitle: "HX"
    title: `${windowTitle} [Wayland置顶] [Wayland隐藏切换]`

    property int bw: 3 // 边框厚度
    property int obscureBw: 10

    property int initWidth: 600
    property int initHeight: 500
    property int initX: (width - initWidth) >> 1
    property int initY: (height - initHeight) >> 1

    property bool closeMask: false
    property bool allowZooming: true // 允许缩放

    // 用户内容组件
    property Component windowItem: Rectangle {
        id: self
        width: 300
        height: 200
        color: "#990099"

        Text {
            anchors.centerIn: parent
            text: "测试"
            color: "white"
        }

        Component.onCompleted: {
            self.x = (root.width - self.width) / 2;
            self.y = (root.height - self.height) / 2;
        }
    }

    // 暴露内部 item 引用
    property alias windowItemRef: loader.item
    property bool isHover: mouseArea.isHover

    function updateMask() {
        resizeMoveHandler.updateMask();
    }

    // 全屏透明层, 用于坐标计算
    Rectangle {
        id: selfSize
        anchors.fill: parent
        color: "transparent"
    }

    // 保证启动时全屏
    Component.onCompleted: {
        root.showFullScreen()
    }

    // 捕捉用户试图退出全屏的行为
    onVisibilityChanged: function(newVisibility) {
        if (newVisibility === Window.Hidden) {
            return;
        }
        if (newVisibility !== Window.FullScreen) {
            Qt.callLater(() => root.showFullScreen())
        }
    }

    signal itemXChanged(val: int);
    signal itemYChanged(val: int);
    signal itemWidthChanged(val: int);
    signal itemHeightChanged(val: int);

    function setRectX(val: int) {
        rect.x = val;
    }

    function setRectY(val: int) {
        rect.y = val;
    }

    function setRectWidth(val: int) {
        rect.width = val;
    }

    function setRectHeight(val: int) {
        rect.height = val;
    }

    // 外层矩形, 作为拖拽 + 缩放边框
    Rectangle {
        id: rect
        x: root.initX
        y: root.initY
        width: root.initWidth
        height: root.initHeight
        color: "transparent"
        border.color: "transparent"
        border.width: root.allowZooming ? 0 : root.bw

        onXChanged: root.itemXChanged(x);
        onYChanged: root.itemYChanged(y);
        onWidthChanged: root.itemWidthChanged(width);
        onHeightChanged: root.itemHeightChanged(height);

        // 内部用户内容
        Loader {
            id: loader
            anchors.fill: parent
            anchors.margins: root.allowZooming ? 0 : root.bw  // 确保内容在边框内部
            sourceComponent: root.windowItem
            onLoaded: resizeMoveHandler.updateMask();
        }

        // 鼠标区域, 用于边框缩放检测
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: !resizeMoveHandler.active && root.allowZooming
            acceptedButtons: Qt.NoButton
            property bool isHover: false
            onEntered: isHover = true
            onExited: isHover = false

            /*
            1  2  3
            4 -1  5
            6  7  8
            */
            property int zoomDirection: -1
            cursorShape: {
                const b = root.bw + root.obscureBw; // 边角判定尺寸
                if (mouseX < b && mouseY < b) {                     // 左上
                    zoomDirection = 1;
                    return Qt.SizeFDiagCursor;
                }
                if (mouseX >= width - b && mouseY >= height - b) {  // 右下
                    zoomDirection = 8;
                    return Qt.SizeFDiagCursor;
                }
                if (mouseX >= width - b && mouseY < b) {            // 右上
                    zoomDirection = 3;
                    return Qt.SizeBDiagCursor;
                }
                if (mouseX < b && mouseY >= height - b) {           // 左下
                    zoomDirection = 6;
                    return Qt.SizeBDiagCursor;
                }
                if (mouseX < b) {                                   // 左
                    zoomDirection = 4;
                    return Qt.SizeHorCursor;
                }
                if (mouseX >= width - b) {                          // 右
                    zoomDirection = 5;
                    return Qt.SizeHorCursor;
                }
                if (mouseY < b) {                                   // 上
                    zoomDirection = 2;
                    return Qt.SizeVerCursor;
                }
                if (mouseY >= height - b) {                         // 下
                    zoomDirection = 7;
                    return Qt.SizeVerCursor;
                }
                zoomDirection = -1;
                return Qt.ArrowCursor;
            }
        }
    }

    // 拖拽与缩放处理
    DragHandler {
        id: resizeMoveHandler
        target: null  // 不直接操作目标
        grabPermissions: PointerHandler.ApprovesTakeOverByAnything

        property bool isResizing: false
        property bool isMoving: false

        property real startX: 0
        property real startY: 0
        property real startWidth: 0
        property real startHeight: 0
        property real startMouseX: 0
        property real startMouseY: 0

        property int resizeZoomDir: -1

        onActiveChanged: {
            if (active) {
                startX = rect.x;
                startY = rect.y;
                startWidth = rect.width;
                startHeight = rect.height;
                startMouseX = centroid.scenePosition.x;
                startMouseY = centroid.scenePosition.y;

                // 保存初始 zoomDirection
                resizeZoomDir = mouseArea.zoomDirection;

                // 判断操作类型
                isResizing = resizeZoomDir !== -1;
                isMoving = !isResizing;

                // 更新掩码初始状态
                updateMask();
            } else {
                resizeZoomDir = -1;
            }
        }

        onTranslationChanged: {
            if (!active) {
                return;
            }

            const dx = centroid.scenePosition.x - startMouseX;
            const dy = centroid.scenePosition.y - startMouseY;

            if (isResizing) {
                handleResize(dx, dy);
            } else if (isMoving) {
                handleMove(dx, dy);
            }

            updateMask();
        }

        function handleMove(dx, dy) {
            // 移动整个矩形
            rect.x = Math.max(0, Math.min(root.width - rect.width, startX + dx));
            rect.y = Math.max(0, Math.min(root.height - rect.height, startY + dy));
        }

        function handleResize(dx, dy) {
            let newX = startX;
            let newY = startY;
            let newWidth = startWidth;
            let newHeight = startHeight;

            switch (resizeZoomDir) {
            case 1: // 左上
                newWidth = Math.max(50, startWidth - dx);
                newHeight = Math.max(50, startHeight - dy);
                newX = startX + (startWidth - newWidth);
                newY = startY + (startHeight - newHeight);
                break;
            case 2: // 上
                newHeight = Math.max(50, startHeight - dy);
                newY = startY + (startHeight - newHeight);
                break;
            case 3: // 右上
                newWidth = Math.max(50, startWidth + dx);
                newHeight = Math.max(50, startHeight - dy);
                newY = startY + (startHeight - newHeight);
                break;
            case 4: // 左
                newWidth = Math.max(50, startWidth - dx);
                newX = startX + (startWidth - newWidth);
                break;
            case 5: // 右
                newWidth = Math.max(50, startWidth + dx);
                break;
            case 6: // 左下
                newWidth = Math.max(50, startWidth - dx);
                newHeight = Math.max(50, startHeight + dy);
                newX = startX + (startWidth - newWidth);
                break;
            case 7: // 下
                newHeight = Math.max(50, startHeight + dy);
                break;
            case 8: // 右下
                newWidth = Math.max(50, startWidth + dx);
                newHeight = Math.max(50, startHeight + dy);
                break;
            }

            // 边界约束
            if (newX < 0) {
                newWidth += newX;
                newX = 0;
            }
            if (newY < 0) {
                newHeight += newY;
                newY = 0;
            }
            if (newX + newWidth > root.width) {
                newWidth = root.width - newX;
            }
            if (newY + newHeight > root.height) {
                newHeight = root.height - newY;
            }

            rect.x = newX;
            rect.y = newY;
            rect.width = newWidth;
            rect.height = newHeight;
        }

        function updateMask() {
            if (root.closeMask) {
                return;
            }
            const posInRoot = rect.mapToItem(selfSize, 0, 0);
            WindowMaskUtil.addControlRect(
                posInRoot.x, posInRoot.y,
                rect.width, rect.height
            );
            WindowMaskUtil.setMask(root);
        }
    }
}