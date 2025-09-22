pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import HX.Music
import "./internal"

Item {
    id: messageManager
    anchors.fill: parent
    focus: true

    // 消息数据模型
    ListModel {
        id: messageModel
    }

    // 单个消息组件
    Component {
        id: messageComponent
        Item {
            focus: true
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
            property int repeatCount: model.repeatCount || 1  // 新增重复计数
            property alias badgeRef: badge
            property alias autoHideTimerRef: autoHideTimer

            // 消息矩形
            Rectangle {
                id: messageRect
                focus: true
                anchors.fill: parent
                radius: 8
                opacity: 0

                // 背景颜色
                color: {
                    return '#60141414';
                    switch (messageItem.messageType) {
                        case "Error": return "#ffebee";
                        case "Warning": return "#fff8e1";
                        case "Success": return "#e8f5e9";
                        default: return "#e3f2fd";
                    }
                }

                // 边框颜色
                border.width: 1
                border.color: {
                    switch (messageItem.messageType) {
                        case "Error": return "#f44336";
                        case "Warning": return "#ffc107";
                        case "Success": return "#4caf50";
                        default: return "#2196f3";
                    }
                }

                // 左侧图标
                MusicActionButton {
                    focus: true
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    width: 28
                    height: 28
                    defaultColor: {
                        switch (messageItem.messageType) {
                            case "Error": return "#d32f2f";
                            case "Warning": return "#f57c00";
                            case "Success": return "#388e3c";
                            default: return "#1976d2";
                        }
                    }
                    url: `qrc:/icons/${(() => {
                        switch (messageItem.messageType) {
                            case "Error": return "error";
                            case "Warning": return "warning";
                            case "Success": return "success";
                            default: return "info";
                        }
                    })()}.svg`
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
                    color: Theme.highlightingColor 
                }

                // 数字角标
                Rectangle {
                    id: badge
                    visible: messageItem.repeatCount > 1
                    width: 20
                    height: 20
                    radius: 10
                    color: {
                        switch (messageItem.messageType) {
                            case "Error": return "#d32f2f";
                            case "Warning": return "#f57c00";
                            case "Success": return "#388e3c";
                            default: return "#1976d2";
                        }
                    }
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.bottomMargin: 8
                    anchors.rightMargin: 8

                    Text {
                        anchors.centerIn: parent
                        text: messageItem.repeatCount
                        font.pixelSize: 12
                        color: "white"
                    }
                }

                // 关闭按钮
                MusicActionButton {
                    focus: true
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 8
                    width: 20
                    height: 20
                    flat: true
                    url: "qrc:/icons/close.svg"
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
                    const targetY = (messageModel.count - 1 - messageItem.messageIndex) *
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
                    deleteTimer.start()
                }
            }

            // 延时删除计时器
            Timer {
                id: deleteTimer
                interval: 10
                onTriggered: {
                    if (messageItem.messageIndex >= 0 && messageItem.messageIndex < messageModel.count) {
                        messageModel.remove(messageItem.messageIndex);
                        messageManager.updateMessagePositions();
                    }
                }
            }

            // 自动隐藏计时器
            Timer {
                id: autoHideTimer
                interval: 3000
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
                focus: false
                propagateComposedEvents: true
                acceptedButtons: Qt.NoButton
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
        focus: true
        width: 300
        height: parent.height

        Repeater {
            focus: true
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

    // 添加消息（带重复角标逻辑）
    function addMessage(type, text) {
        for (let i = 0; i < messageModel.count; ++i) {
            if (messageModel.get(i).messageText === text) {
                // 已存在相同消息, 数字角标加1, 重置计时器
                const existingItem = messageContainer.children[i];
                if (existingItem) {
                    existingItem.repeatCount += 1;
                    existingItem.autoHideTimerRef.stop();
                    existingItem.autoHideTimerRef.start(); // 重置计时器
                    existingItem.badgeRef.visible = true;
                }
                return; // 不新增消息
            }
        }

        // 新增消息
        messageModel.append({
            "messageType": type,
            "messageText": text,
            "repeatCount": 1
        })

        updateMessagePositions();
    }

    // 更新所有消息的位置
    function updateMessagePositions() {
        for (let i = 0; i < messageModel.count; ++i) {
            const currentItem = messageContainer.children[i];
            if (currentItem) {
                const targetY = (messageModel.count - 1 - i) * (currentItem.height + 10);

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
