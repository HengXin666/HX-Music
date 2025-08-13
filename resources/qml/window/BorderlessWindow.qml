pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: root
    flags: Qt.FramelessWindowHint
    color: "transparent"

    property int bw: 3                  // 边框厚度
    property bool allowInnerMove: true  // 中间区域是否允许拖动
    property bool showBorder: true      // 是否显示边框 (不影响resize交互)
    property color borderColor: "red"   // 边框颜色
    property Component delegate: Item { // 正文内容
        Text {
            text: "请务必实现 delegate 自绘内部内容"
        }
    }
    property Component titleBar: Rectangle { // 自绘标题栏, 可为null, 内部需要自定义 height
        height: 30
        color: "#3f3f3f"
        MouseArea {
            anchors.fill: parent
            onDoubleClicked: root.toggleMaximized()
            acceptedButtons: Qt.LeftButton
            // 不要拖动, 移动逻辑交给外面的 DragHandler
        }
        RowLayout {
            anchors.fill: parent
            Label {
                text: root.title
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            // 最小化按钮
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Image {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "qrc:/icons/dropdown.svg"
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.showMinimized()
                }
            }

            // 最大化/还原按钮
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Image {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: root.visibility === Window.Maximized
                            ? "qrc:/icons/restore.svg"   // 还原图标
                            : "qrc:/icons/up.svg"  // 最大化图标
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (root.visibility === Window.Maximized)
                            root.showNormal()
                        else
                            root.showMaximized()
                    }
                }
            }

            // 关闭按钮
            Item {
                Layout.preferredWidth: 30
                Layout.preferredHeight: 30
                Image {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    source: "qrc:/icons/close.svg" // 你自己换
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: root.close()
                }
            }
        }
    }

    function toggleMaximized() {
        if ((root.visibility & Window.Maximized) === Window.Maximized) {
            root.showNormal();
        } else {
            root.showMaximized();
        }
    }

    // 内部状态数据
    QtObject {
        id: self
        property int bw: 0 // 边框厚度 [const]

        Component.onCompleted: {
            bw = root.bw;
        }
    }

    onVisibilityChanged: (val) => {
        if (self.bw === 0) {
            return;
        }
        root.bw = (val & Window.Maximized) === Window.Maximized 
            ? 0 
            : self.bw;
    }

    // 鼠标区域仅用于设置正确的光标形状
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: {
            const mousePoint = Qt.point(mouseX, mouseY);
            const b = root.bw + 10; // 角尺寸
            if (mousePoint.x < b && mousePoint.y < root.bw)              // 左上
                return Qt.SizeFDiagCursor;
            if (mousePoint.x >= width - b && mousePoint.y >= height - b) // 右下
                return Qt.SizeFDiagCursor;
            if (mousePoint.x >= width - b && mousePoint.y < root.bw)     // 右上
                return Qt.SizeBDiagCursor;
            if (mousePoint.x < b && mousePoint.y >= height - b)          // 左下
                return Qt.SizeBDiagCursor;
            if (mousePoint.x < b || mousePoint.x >= width - b)           // 水平
                return Qt.SizeHorCursor;
            if (mousePoint.y < root.bw || mousePoint.y >= height - b)    // 竖直
                return Qt.SizeVerCursor;
        }
        acceptedButtons: Qt.NoButton // 不处理实际事件
    }

    // 缩放
    DragHandler {
        id: resizeHandler
        target: null
        // 覆盖整个窗口
        onActiveChanged: if (active) {
            const p = resizeHandler.centroid.position;
            const b = root.bw + 10; // 角尺寸
            let e = 0;
            const leftEdge = p.x < b;
            const rightEdge = p.x >= root.width - b;
            const topEdge = p.y < root.bw;
            const bottomEdge = p.y >= root.height - b;

            if (leftEdge) {
                e |= Qt.LeftEdge;
            }
            if (rightEdge) {
                e |= Qt.RightEdge;
            }
            if (topEdge) {
                e |= Qt.TopEdge;
            }
            if (bottomEdge) {
                e |= Qt.BottomEdge;
            }

            if (e !== 0) {
                root.startSystemResize(e);
            } else if (root.allowInnerMove) {
                root.startSystemMove();
            }
        }
    }

    // 绘制边框 (仅视觉)
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: root.showBorder ? root.borderColor : "transparent"
        border.width: root.showBorder ? root.bw : 0
    }

    // 标题栏 (如果有的话)
    Loader {
        width: parent.width - 2 * root.bw
        x: root.bw
        y: root.bw
        id: titleBarLoader
        sourceComponent: root.titleBar
    }

    // 内容区
    Loader {
        x: root.bw
        y: titleBarLoader.item ? titleBarLoader.y + titleBarLoader.height : root.bw
        width: root.width - 2 * root.bw
        height: root.height - (titleBarLoader.item ? titleBarLoader.y + titleBarLoader.height + root.bw : 2 * root.bw)
        sourceComponent: root.delegate
    }
}