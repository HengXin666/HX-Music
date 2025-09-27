pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import HX.Music
import "qrc:/HX/Music/qml/widget/internal"

Item {
    id: root
    ScrollView {
        id: scrollArea
        anchors.fill: parent
        clip: true

        // 解决 rightPadding 循环绑定问题, 不直接依赖 ScrollBar.width
        rightPadding: 12

        ColumnLayout {
            id: contentLayout
            width: parent.width
            spacing: 20
            anchors.margins: 20

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: "服务器设置"
                font.pixelSize: 24
                color: Theme.highlightingColor
                Layout.alignment: Qt.AlignCenter
            }

            // 音乐扫描
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                // 自定义标题
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Theme.paratextColor
                        opacity: 0.3
                    }

                    Text {
                        text: "音乐扫描"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                // 音乐扫描
                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignLeft

                    Label {
                        text: "扫描音乐库, 把没有添加到音乐库的音乐添加进去"
                        color: Theme.textColor
                        font.pixelSize: 16
                    }
                    TextButton {
                        text: "开始扫描"
                        onClicked: MusicController.startScan()
                    }
                }
            }

            // 歌词爬取
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                // 自定义标题
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Theme.paratextColor
                        opacity: 0.3
                    }

                    Text {
                        text: "歌词爬取"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                // 歌词爬取
                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignLeft

                    Label {
                        text: "扫描音乐库, 并且为没有歌词的音乐爬取歌词"
                        color: Theme.textColor
                        font.pixelSize: 16
                    }
                    TextButton {
                        text: "开始扫描"
                        onClicked: LyricController.startScan()
                    }
                }
            }

            // 实时日志
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                // 自定义标题
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Theme.paratextColor
                        opacity: 0.3
                    }

                    Text {
                        text: "实时日志"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                // 日志显示区域, 通过信号更新内容, 支持滚动
                ScrollView {
                    id: logScrollView
                    Layout.fillWidth: true
                    Layout.preferredHeight: 300
                    clip: true

                    ScrollBar.vertical.policy: ScrollBar.AsNeeded
                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded

                    TextArea {
                        id: logArea
                        readOnly: true
                        wrapMode: TextArea.WrapAnywhere
                        font.pixelSize: 14
                        color: Theme.textColor
                        background: Rectangle {
                            color: Theme.backgroundColor.darker(110)
                            border.width: 1
                            border.color: Theme.paratextColor
                            radius: 6
                        }

                        // 自动滚动到底部
                        onTextChanged: {
                            logScrollView.contentItem.contentY = logScrollView.contentItem.contentHeight - logScrollView.height;
                        }

                        // 监听日志更新信号
                        Connections {
                            target: SignalBusSingleton
                            function onBackendViewLogUpdated(log: string) {
                                logArea.append(log + "\n");
                            }
                        }
                    }
                }

                // 清空日志按钮
                TextButton {
                    text: "清空日志"
                    Layout.alignment: Qt.AlignRight
                    onClicked: {
                        logArea.text = "";
                    }
                }
            }

            // 添加用户
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                // 自定义标题
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Theme.paratextColor
                        opacity: 0.3
                    }

                    Text {
                        text: "添加用户"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                // 添加用户
                ColumnLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignLeft

                    IconTextField {
                        id: userNameField
                        Layout.alignment: Qt.AlignLeft
                        Layout.preferredWidth: 230
                        iconDefaultColor: Theme.textColor
                        iconHighlightColor: Theme.highlightingColor
                        borderHighlightColor: Theme.highlightingColor
                        iconSource: "qrc:/icons/user.svg"
                        placeholderText: "用户名"
                        KeyNavigation.tab: password1Field
                        onAccepted: password2Field.accepted()
                    }
                    IconTextField {
                        id: password1Field
                        Layout.alignment: Qt.AlignLeft
                        Layout.preferredWidth: 230
                        iconDefaultColor: Theme.textColor
                        iconHighlightColor: Theme.highlightingColor
                        borderHighlightColor: Theme.highlightingColor
                        iconSource: "qrc:/icons/lock.svg"
                        placeholderText: "密码"
                        isPasswordMode: true
                        clickableIconSource: "qrc:/icons/unlock.svg"
                        onIconClicked: (isDefault) => {
                            isPasswordMode = isDefault;
                        }
                        KeyNavigation.tab: password2Field
                        onAccepted: password2Field.accepted()
                    }
                    IconTextField {
                        id: password2Field
                        Layout.alignment: Qt.AlignLeft
                        Layout.preferredWidth: 230
                        iconDefaultColor: Theme.textColor
                        iconHighlightColor: Theme.highlightingColor
                        borderHighlightColor: Theme.highlightingColor
                        iconSource: "qrc:/icons/lock.svg"
                        placeholderText: "再次输入密码"
                        isPasswordMode: true
                        clickableIconSource: "qrc:/icons/unlock.svg"
                        onIconClicked: (isDefault) => {
                            isPasswordMode = isDefault;
                        }
                        onAccepted: {
                            if (userNameField.text.length === 0) {
                                MessageController.showError("用户名不能为空");
                                return;
                            }
                            if (password1Field.text.length === 0) {
                                MessageController.showError("密码不能为空");
                                return;
                            }
                            if (password1Field.text !== password2Field.text) {
                                MessageController.showError("两次输入的新密码不一致");
                                return;
                            }
                            UserController.addUser(userNameField.text, password1Field.text, levelComboBox.currentIndex);
                        }
                    }

                    RowLayout {
                        spacing: 8
                        Layout.alignment: Qt.AlignLeft
                        
                        Label {
                            text: "用户权限"
                            color: Theme.textColor
                            font.pixelSize: 16
                        }

                        ComboBox {
                            id: levelComboBox
                            model: ["管理员", "普通用户", "只读用户"]

                            // 设置默认显示文字颜色
                            contentItem: Text {
                                text: levelComboBox.displayText
                                font.pixelSize: 16
                                color: Theme.textColor  // 指定文字颜色
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    TextButton {
                        text: "新增用户"
                        onClicked: password2Field.accepted()
                    }
                }
            }

            // 删除用户
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                // 自定义标题
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 1
                        color: Theme.paratextColor
                        opacity: 0.3
                    }

                    Text {
                        text: "删除用户"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                // 删除用户
                ColumnLayout {
                    spacing: 12
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    RowLayout {
                        TextButton {
                            text: "刷新列表"
                            Layout.alignment: Qt.AlignLeft
                            onClicked: userListView.refreshUserList()
                        }
                        // 空状态提示
                        Label {
                            text: `用户总数: ${userListView.count}`
                            color: Theme.textColor
                            font.pixelSize: 16
                        }
                    }

                    // 删除用户
                    Rectangle {
                        Layout.preferredWidth: root.width - 20
                        Layout.preferredHeight: 300
                        radius: 8
                        color: Theme.backgroundColor

                        // 表头
                        RowLayout {
                            id: tableHead
                            spacing: 5
                            height: 30

                            Text {
                                text: "用户ID"
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textColor
                                Layout.preferredWidth: 80
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                text: "用户名"
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textColor
                                Layout.preferredWidth: 240
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                text: "创建歌单数量"
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textColor
                                Layout.preferredWidth: 100
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                text: "收藏歌单数量"
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textColor
                                Layout.alignment: Qt.AlignCenter
                                Layout.preferredWidth: 100
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Text {
                                text: "权限级别"
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textColor
                                Layout.preferredWidth: 100
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }

                            Item {
                                Layout.fillWidth: true
                            }

                            Text {
                                text: "操作"
                                font.pixelSize: 14
                                font.bold: true
                                color: Theme.textColor
                                Layout.preferredWidth: 80
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        // 用户列表
                        ListView {
                            id: userListView
                            clip: true
                            spacing: 5
                            anchors.top: tableHead.bottom
                            anchors.topMargin: 5
                            anchors.leftMargin: 8
                            anchors.rightMargin: 8
                            anchors.bottomMargin: 8
                            width: parent.width - 2 * anchors.leftMargin
                            height: 300

                            model: ListModel {
                                id: userListModel
                            }
                            
                            ScrollBar.vertical: ScrollBar {
                                policy: ScrollBar.AsNeeded
                                width: 12
                            }

                            delegate: Rectangle {
                                id: delegateRoot
                                required property int userId
                                required property string name
                                required property int createdPlaylistLen
                                required property int savedPlaylistLen
                                required property int permissionLevel
                                required property int index
                                width: userListView.width
                                height: 36
                                color: "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    Text {
                                        text: `[${delegateRoot.userId}]`
                                        font.pixelSize: 14
                                        color: Theme.textColor
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                        Layout.preferredWidth: 80
                                    }

                                    Text {
                                        text: delegateRoot.name
                                        font.pixelSize: 14
                                        color: Theme.textColor
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                        Layout.preferredWidth: 240
                                    }

                                    Text {
                                        text: delegateRoot.createdPlaylistLen
                                        font.pixelSize: 14
                                        color: Theme.textColor
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.preferredWidth: 100
                                    }

                                    Text {
                                        text: delegateRoot.savedPlaylistLen
                                        font.pixelSize: 14
                                        color: Theme.textColor
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.preferredWidth: 100
                                    }

                                    Text {
                                        text: {
                                            switch (delegateRoot.permissionLevel) {
                                                case 0: return "管理员";
                                                case 1: return "普通用户";
                                                case 2: return "只读用户";
                                                default: return "未知";
                                            }
                                        }
                                        font.pixelSize: 14
                                        color: {
                                            switch (delegateRoot.permissionLevel) {
                                                case 0: return "#ff6b6b"; // 红色表示管理员
                                                case 1: return Theme.textColor;
                                                case 2: return "#4ecdc4"; // 青色表示只读用户
                                                default: return Theme.textColor;
                                            }
                                        }
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                        Layout.preferredWidth: 100
                                    }

                                    Button {
                                        text: "删除"
                                        Layout.preferredWidth: 80
                                        background: Rectangle {
                                            color: parent.hovered ? "#ff6b6b" : Theme.backgroundColor
                                            radius: 4
                                            border.width: 1
                                            border.color: parent.hovered ? "#ff6b6b" : Theme.paratextColor
                                        }
                                        onClicked: {
                                            // MessageController.showWarning(`真的要删除 [${delegateRoot.name}] 吗? 5 秒内再次点击删除以删除用户!`);
                                            UserController.delUser(delegateRoot.userId);
                                        }
                                    }
                                }
                            }

                            function refreshUserList() {
                                UserController.getUserInfoList();
                            }

                            Component.onCompleted: {
                                Qt.callLater(() => refreshUserList());
                            }

                            Connections {
                                target: UserController
                                function onUpdateUserInfoList(list) {
                                    userListModel.clear();
                                    
                                    for (let i = 0; i < list.length; ++i) {
                                        const v = list[i];
                                        userListModel.append({
                                            userId: parseInt(v.userId),
                                            name: v.name,
                                            createdPlaylistLen: parseInt(v.createdPlaylistLen),
                                            savedPlaylistLen: parseInt(v.savedPlaylistLen),
                                            permissionLevel: parseInt(v.permissionLevel)
                                        });
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}