pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Window 2.3
import QtQuick.Controls
import HX.Music

Window {
    id: root
    visible: true
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"
    width: Screen.width
    height: Screen.height

    property string windowTitle: "HX"
    title: `${windowTitle} [Wayland置顶]`

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

    // 暴露的引用
    property alias windowItemRef: loader.item

    Rectangle {
        id: selfSize
        anchors.fill: parent
        color: "transparent"
    }

    Loader {
        id: loader
        sourceComponent: root.windowItem
        onLoaded: {
            const item = loader.item as Item;
            if (item) {
                item.parent = selfSize;
                Qt.callLater(() => {
                    const posInRoot = item.mapToItem(selfSize, 0, 0);
                    WindowMaskUtil.clear(root);
                    WindowMaskUtil.addControlRect(posInRoot.x, posInRoot.y, item.width, item.height);
                    WindowMaskUtil.setMask(root);
                });
            }
        }
    }

    DragHandler {
        target: root.windowItemRef as Item

        xAxis {
            enabled: true
            minimum: 0
            maximum: root.width - target.width
        }

        yAxis {
            enabled: true
            minimum: 0
            maximum: root.height - target.height
        }

        onActiveChanged: if (active) {
            updateMask();
        }

        onTranslationChanged: {
            // 每次拖动时都更新掩码
            updateMask();
        }

        function updateMask() {
            const item = root.windowItemRef as Item;
            const posInRoot = item.mapToItem(selfSize, 0, 0);
            WindowMaskUtil.clear(root);
            WindowMaskUtil.addControlRect(posInRoot.x, posInRoot.y, item.width, item.height);
            WindowMaskUtil.setMask(root);
        }
    }
}
