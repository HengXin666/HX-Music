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
            
            Button {
                text: "清空已完成"
                onClicked: {
                    // 这里需要实现清空已完成项目的逻辑
                    console.log("清空已完成点击")
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

                property int index: index
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
                                        return "red";
                                    }
                                }
                                
                                Item { Layout.fillWidth: true }
                                
                                Label {
                                    text: {
                                        if (delegateItem.model.uploadSpeed === 0) 
                                            return "";
                                        const speed = delegateItem.model.uploadSpeed / 1024;
                                        if (speed < 1024) 
                                            return speed.toFixed(1) + " KB/s";
                                        return (speed / 1024).toFixed(1) + " MB/s";
                                    }
                                    font.pixelSize: 12
                                    color: "gray"
                                    visible: delegateItem.model.uploadStatus === 1
                                }
                            }
                        }
                        
                        // 操作按钮
                        Button {
                            text: {
                                if (delegateItem.model.uploadStatus === 0) 
                                    return "开始"
                                if (delegateItem.model.uploadStatus === 1) 
                                    return "暂停"
                                return "打开"
                            }
                            onClicked: {
                                console.log("操作按钮点击:", delegateItem.index, delegateItem.model.name)
                                // 这里需要实现开始/暂停/打开等操作
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
                text: {
                    const total = listView.count;
                    const completed = 0; // 这里需要计算已完成的数量
                    return `总计: ${total} 已完成: ${completed}`;
                }
            }
            
            Item { Layout.fillWidth: true }
            
            Label {
                text: {
                    // 这里可以显示总上传速度
                    return "总速度: 0 KB/s"
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
                // 默认歌单ID为0，实际应用中可能需要用户选择
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
}