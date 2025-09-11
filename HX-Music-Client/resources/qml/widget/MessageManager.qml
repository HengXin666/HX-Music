pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import HX.Music

Item {
    id: messageManager
    anchors.fill: parent
    z: 9999 // 确保消息显示在最上层

    // 消息数据模型
    ListModel {
        id: messageModel
    }

    // 单个消息组件
    Component {
        id: messageComponent
        Item {
            id: messageItem
            width: 300
            height: 70
            x: parent.width
            y: 0

            required property int index
            required property var model

            property string messageType: model.messageType
            property string messageText: model.messageText
            property int messageIndex: index

            // 消息矩形
            Rectangle {
                id: messageRect
                anchors.fill: parent
                radius: 8
                opacity: 0

                // 背景颜色
                color: {
                    switch (messageType) {
                        case "error": return "#ffebee";
                        case "warning": return "#fff8e1";
                        case "success": return "#e8f5e9";
                        default: return "#e3f2fd";
                    }
                }

                // 边框颜色
                border.width: 1
                border.color: {
                    switch (messageType) {
                        case "error": return "#f44336";
                        case "warning": return "#ffc107";
                        case "success": return "#4caf50";
                        default: return "#2196f3";
                    }
                }

                // 左侧图标
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "Material Icons"
                    font.pixelSize: 24
                    text: {
                        switch (messageItem.messageType) {
                            case "error": return "\ue000";
                            case "warning": return "\ue002";
                            case "success": return "\ue86c";
                            default: return "\ue88e";
                        }
                    }
                    color: {
                        switch (messageItem.messageType) {
                            case "error": return "#d32f2f";
                            case "warning": return "#f57c00";
                            case "success": return "#388e3c";
                            default: return "#1976d2";
                        }
                    }
                }

                // 消息文本
                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 50
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: messageItem.messageText
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    font.pixelSize: 14
                    color: "#212121"
                }

                // 关闭按钮
                Button {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    width: 30
                    height: 30
                    flat: true
                    icon.source: "qrc:/icons/close.svg"
                    icon.color: "#757575"
                    onClicked: disappearAnimation.start()
                }
            }

            // 出现动画
            ParallelAnimation {
                id: appearAnimation
                NumberAnimation {
                    target: messageRect
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 300
                }
                NumberAnimation {
                    target: messageItem
                    property: "x"
                    from: messageContainer.width
                    to: messageContainer.width - messageItem.width - 10
                    duration: 400
                    easing.type: Easing.OutCubic
                }
                onStarted: {
                    // 在动画开始时计算并设置Y位置
                    var targetY = (messageModel.count - 1 - messageItem.messageIndex) *
                                 (messageItem.height + 10);
                    messageItem.y = targetY;
                }
            }

            // 消失动画
            ParallelAnimation {
                id: disappearAnimation
                NumberAnimation {
                    target: messageRect
                    property: "opacity"
                    to: 0
                    duration: 300
                }
                NumberAnimation {
                    target: messageItem
                    property: "x"
                    to: messageContainer.width
                    duration: 300
                }
                onFinished: {
                    // 使用延时删除, 避免在动画过程中修改模型
                    deleteTimer.start()
                }
            }

            // 延时删除计时器
            Timer {
                id: deleteTimer
                interval: 10
                onTriggered: {
                    if (messageItem.messageIndex >= 0 && messageItem.messageIndex < messageModel.count) {
                        messageModel.remove(messageItem.messageIndex)
                    }
                }
            }

            // 自动隐藏计时器
            Timer {
                id: autoHideTimer
                interval: 2000
                repeat: false
                onTriggered: disappearAnimation.start()
            }

            // 组件初始化
            Component.onCompleted: {
                appearAnimation.start()
                autoHideTimer.start()
            }

            // 鼠标悬停时暂停自动隐藏
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: autoHideTimer.stop()
                onExited: autoHideTimer.start()
            }
        }
    }

    // 消息容器
    Item {
        id: messageContainer
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: 10
        }
        width: 300
        height: parent.height

        Repeater {
            model: messageModel
            delegate: messageComponent
        }
    }

    // 监听 C++ 信号
    Connections {
        target: MessageController
        function onShowMessageRequested(type, message) {
            messageManager.addMessage(type, message)
        }
    }

    // 添加消息
    function addMessage(type, text) {
        // 先添加消息到模型
        messageModel.append({
            "messageType": type,
            "messageText": text
        })

        // 如果有太多消息, 移除最早的消息
        // if (messageModel.count > 8) {
        //     messageModel.remove(0);
        // }

        // 更新所有消息的位置
        updateMessagePositions();
    }

    // 更新所有消息的位置
    function updateMessagePositions() {
        for (let i = 0; i < messageModel.count; ++i) {
            const currentItem = messageContainer.children[i];
            if (currentItem) {
                const targetY = (messageModel.count - 1 - i) * (currentItem.height + 10);

                // 创建并启动Y位置动画
                const yAnimation = Qt.createQmlObject(`
                    import QtQuick 2.0
                    NumberAnimation {
                        property: "y"
                        to: ${targetY}
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                `, currentItem);

                yAnimation.target = currentItem;
                yAnimation.start();
            }
        }
    }
}