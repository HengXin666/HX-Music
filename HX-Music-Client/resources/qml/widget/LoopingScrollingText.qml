pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts

Item {
    id: root
    height: 40

    property var itemData: []       // 用户数据 {text, onClick}
    property real scrollSpeed: 50   // 像素/秒
    property real spacingWidth: 75  // 间隔宽度
    property bool isRunging: true   // 外界控制是否移动 (移动开关)

    property bool _hovered: false
    property real _contentWidth: mainView.implicitWidth  // 实际内容宽度
    // 计算是否需要滚动(内容超出容器宽度时才滚动)
    property bool _needScrolling: isRunging && _contentWidth > width

    // 默认委托(可被外部覆盖)
    default property alias contentDelegate: repeater.delegate

    // 轻量委托
    property color textColor: "#f6f6f6"
    property int fontPixelSize: 16

    function checkWidth() {
        scrollAnimation.stop();
        flickable.contentX = 0;

        // 更新动画参数
        scrollAnimation.from = 0;
        scrollAnimation.to = root._contentWidth + root.spacingWidth;
        scrollAnimation.duration = (scrollAnimation.to / root.scrollSpeed) * 1000;

        if (root._needScrolling && !root._hovered) {
            scrollAnimation.start();
        }
    }

    // 检测窗口变化, 防止隐藏时候没有更新, 而显示时候出现bug
    Connections {
        target: root.Window
        function onActiveChanged() {
            root.checkWidth();
        }
        function onVisibilityChanged() {
            root.checkWidth();
        }
    }

    on_ContentWidthChanged: {
        Qt.callLater(() => root.checkWidth());
    }

    // 背景容器
    Rectangle {
        anchors.fill: parent
        color: "transparent"

        // 悬停检测区域
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: root._hovered = true
            onExited: root._hovered = false
        }

        // 滚动视图
        Flickable {
            id: flickable
            anchors.fill: parent
            contentWidth: row.implicitWidth  // 实际内容宽度
            contentHeight: height
            clip: true
            interactive: false  // 禁用手动交互

            // 循环滚动内容(两组相同内容实现无缝衔接)
            RowLayout {
                id: row
                spacing: root.spacingWidth

                // 第一组内容
                RowLayout {
                    id: mainView
                    Repeater {
                        id: repeater
                        model: root.itemData
                        delegate: MouseArea {
                            id: mainViewItem
                            required property var modelData

                            width: textItem.implicitWidth
                            height: root.height
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            // 点击事件处理
                            onClicked: {
                                if (modelData && modelData.onClick) {
                                    modelData.onClick();
                                }
                            }

                            // 文本显示
                            Text {
                                id: textItem
                                text: mainViewItem.modelData.text || ""
                                color: root.textColor
                                font.pixelSize: root.fontPixelSize
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }

                // 第二组内容
                RowLayout {
                    id: copyView
                    Repeater {
                        model: root.itemData
                        delegate: MouseArea {
                            id: copyViewItem
                            required property var modelData

                            visible: root._needScrolling
                            width: textItem02.implicitWidth
                            height: root.height
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            // 点击事件处理
                            onClicked: {
                                if (modelData && modelData.onClick) {
                                    modelData.onClick();
                                }
                            }

                            // 文本显示
                            Text {
                                id: textItem02
                                visible: root._needScrolling
                                text: copyViewItem.modelData.text || ""
                                color: root.textColor
                                font.pixelSize: root.fontPixelSize
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }

            // 循环滚动动画
            NumberAnimation {
                id: scrollAnimation
                target: flickable
                property: "contentX"
                loops: Animation.Infinite
                from: 0
                to: root._contentWidth + root.spacingWidth
                duration: (to / root.scrollSpeed) * 1000
                running: false
            }

            // 鼠标悬浮控制动画暂停/恢复
            MouseArea {
                acceptedButtons: Qt.NoButton
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    root._hovered = true;
                    if (scrollAnimation.running) {
                        scrollAnimation.pause();
                    }
                }
                onExited: {
                    root._hovered = false;
                    if (root._needScrolling) {
                        // 如果之前是暂停状态, 那么继续
                        if (scrollAnimation.paused) {
                            scrollAnimation.resume();
                        } else {
                            // 如果是第一次滚动, 启动动画
                            scrollAnimation.start();
                        }
                    }
                }
            }
        }

        onWidthChanged: {
            root.checkWidth();
        }

        // 初始化后重置位置
        Component.onCompleted: {
            flickable.contentX = 0;
            root.checkWidth();
        }
    }
}
