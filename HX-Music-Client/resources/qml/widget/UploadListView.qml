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

    // 上传列表模型
    UploadListModel {
        id: uploadListModel
    }

    // 主布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        // 标题区域
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "上传列表"
                font.bold: true
                font.pixelSize: 18
                Layout.fillWidth: true
            }

            Switch {
                id: autoCreatePlaylistSwitch
                text: "自动创建歌单 (上传文件夹)"
                checked: true
                onCheckedChanged: {
                    uploadListModel.setAutomaticallyCreatePlaylist(checked);
                }
            }

            Button {
                text: "清空已完成"
                onClicked: {
                    // 这里需要实现清空已完成项目的逻辑
                    MessageController.showInfo("已清空已完成任务");
                    uploadListModel.clearCompletedTask();
                }
            }
        }

        // 上传列表
        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: uploadListModel
            spacing: 5

            delegate: Item {
                id: delegateItem
                width: listView.width
                height: 70

                required property int index
                required property var model

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: 5

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 15

                        // 文件图标
                        Rectangle {
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 40
                            radius: 5
                            color: Theme.backgroundColor

                            Label {
                                anchors.centerIn: parent
                                text: {
                                    const ext = delegateItem.model.name.split('.').pop().toUpperCase()
                                    return ext.length <= 4 ? ext : "FILE"
                                }
                                color: Theme.highlightingColor
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }

                        // 文件信息和进度
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 5

                            Label {
                                text: delegateItem.model.name
                                color: Theme.textColor
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                font.bold: true
                            }

                            ProgressBar {
                                value: delegateItem.model.progress / 100
                                Layout.fillWidth: true
                                visible: delegateItem.model.uploadStatus !== 2
                            }

                            RowLayout {
                                Layout.fillWidth: true

                                Label {
                                    text: {
                                        if (delegateItem.model.uploadStatus === 0)
                                            return "等待上传";
                                        if (delegateItem.model.uploadStatus === 1)
                                            return "上传中";
                                        if (delegateItem.model.uploadStatus === 2)
                                            return "上传完成";
                                        if (delegateItem.model.uploadStatus === 3)
                                            return "上传失败: " + delegateItem.model.errMsg;
                                        if (delegateItem.model.uploadStatus === 4)
                                            return "暂停中";
                                        return "未知状态";
                                    }
                                    font.pixelSize: 12
                                    color: {
                                        if (delegateItem.model.uploadStatus === 0)
                                            return "gray";
                                        if (delegateItem.model.uploadStatus === 1)
                                            return "blue";
                                        if (delegateItem.model.uploadStatus === 2)
                                            return "green";
                                        if (delegateItem.model.uploadStatus === 3)
                                            return "red";
                                        if (delegateItem.model.uploadStatus === 4)
                                            return "orange";
                                        return "red";
                                    }
                                }

                                Item { Layout.fillWidth: true }

                                Label {
                                    text: `( ${root.formatSize(delegateItem.model.uploadSpeed)}/s )`
                                    font.pixelSize: 12
                                    color: Theme.highlightingColor
                                    visible: delegateItem.model.uploadStatus === 1
                                }

                                Label {
                                    text: `${root.formatSize(delegateItem.model.nowUploadSize)} / ${root.formatSize(delegateItem.model.totalSize)}`
                                    font.pixelSize: 12
                                    color: "gold"
                                }
                            }
                        }

                        // 操作按钮
                        Button {
                            text: {
                                if (delegateItem.model.uploadStatus === 0)
                                    return "开始";
                                if (delegateItem.model.uploadStatus === 1)
                                    return "暂停";
                                if (delegateItem.model.uploadStatus === 2)
                                    return "完成";
                                if (delegateItem.model.uploadStatus === 3)
                                    return "重试";
                                if (delegateItem.model.uploadStatus === 4)
                                    return "继续";
                                return "删除";
                            }
                            onClicked: {
                                if (delegateItem.model.uploadStatus === 1) {
                                    MessageController.showInfo("已暂停");
                                    uploadListModel.stopTask(delegateItem.index);
                                } else if (delegateItem.model.uploadStatus === 3
                                        || delegateItem.model.uploadStatus === 4
                                ) {
                                    MessageController.showInfo(delegateItem.model.uploadStatus === 3 ? "继续上传" : "重试上传");
                                    uploadListModel.resumeTask(delegateItem.index);
                                } else if (delegateItem.model.uploadStatus === 2) {
                                    MessageController.showSuccess("上传已完成!");
                                } else if (delegateItem.model.uploadStatus === 0) {
                                    MessageController.showInfo("开始上传");
                                    uploadListModel.resumeTask(delegateItem.index);
                                }
                            }
                        }
                    }
                }
            }

            // 空列表提示
            Label {
                id: emptyHint
                anchors.centerIn: parent
                text: "拖拽文件到此处上传"
                color: Theme.textColor
                font.pixelSize: 16
                visible: listView.count === 0
            }
        }

        // 底部状态栏
        RowLayout {
            Layout.fillWidth: true

            Label {
                id: statusLabel
                text: {
                    const total = listView.count;
                    const completed = 0;
                    return `总计: ${total} 已完成: ${completed}`;
                }

                // 绑定信号
                Connections {
                    target: uploadListModel
                    function onUpdateTaskCnt() {
                        const total = listView.count;
                        const completed = uploadListModel.getTaskCnt();
                        statusLabel.text = `总计: ${total} 已完成: ${completed}`;
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Label {
                id: speedLabel
                text: "0 B/s"
                // 绑定信号
                Connections {
                    target: uploadListModel
                    function onUpdateTotalUploadSpeed() {
                        speedLabel.text = root.formatSize(uploadListModel.getTotalUploadSpeed()) + "/s";
                    }
                }
            }
        }
    }

    // 拖拽区域 - 支持拖拽文件上传
    DropArea {
        anchors.fill: parent
        z: 5
        onEntered: drag => {
            root.opacity = 0.9;
            drag.accept(Qt.CopyAction);
            dropHint.visible = true;
            emptyHint.visible = false;
        }
        onExited: {
            root.opacity = 1
            dropHint.visible = false;
            emptyHint.visible = listView.count === 0;
        }
        onDropped: drop => {
            root.opacity = 1;
            dropHint.visible = false;
            emptyHint.visible = false;
            for (const urlStr of drop.urls) {
                const path = decodeURIComponent(urlStr).replace("file://", "")
                // 默认歌单ID为0, 
                uploadListModel.uploadFile(path, 0)
            }
        }
    }

    // 拖拽提示效果
    Rectangle {
        id: dropHint
        anchors.fill: parent
        color: '#4b00aaff'
        visible: false
        border.width: 3
        border.color: "#0000ff"
        radius: 5
        z: 10

        Label {
            anchors.centerIn: parent
            text: "松开: 上传文件"
            font.pixelSize: 24
            color: "white"
        }
    }

    // 计算进制
    function formatSize(size) {
        if (size < 1024)
            return size + " B";
        if (size < 1024 * 1024)
            return (size / 1024).toFixed(1) + " KB";
        if (size < 1024 * 1024 * 1024)
            return (size / (1024 * 1024)).toFixed(1) + " MB";
        return (size / (1024 * 1024 * 1024)).toFixed(1) + " GB";
    }
}