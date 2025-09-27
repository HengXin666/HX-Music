import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects
import HX.Music
import "../widget/internal"

Item {
    id: root
    ScrollView {
        id: scrollArea
        anchors.fill: parent
        clip: true

        // 解决 rightPadding 循环绑定问题, 不直接依赖 ScrollBar.width
        rightPadding: 12

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOn
            interactive: true
            // 使用 implicitWidth 避免和 ScrollView 相互绑定
            implicitWidth: 12
        }

        ColumnLayout {
            id: contentLayout
            width: parent.width
            spacing: 20
            anchors.margins: 20

            Item {
                Layout.fillWidth: true
            }

            Text {
                text: "用户设置"
                font.pixelSize: 24
                color: Theme.highlightingColor
                Layout.alignment: Qt.AlignCenter
            }

            // 用户信息
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
                        text: "用户信息"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                Text {
                    text: "修改用户名"
                    color: Theme.textColor
                    font.pixelSize: 16
                }

                // 用户名
                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignLeft

                    IconTextField {
                        id: usernameField
                        Layout.alignment: Qt.AlignRight
                        Layout.preferredWidth: 230
                        iconDefaultColor: Theme.textColor
                        iconHighlightColor: Theme.highlightingColor
                        borderHighlightColor: Theme.highlightingColor
                        iconSource: "qrc:/icons/user.svg"
                        placeholderText: "新用户名"
                        text: UserController.getName()
                        onAccepted: {
                            if (text.length === 0) {
                                MessageController.showError("用户名不能为空");
                                return;
                            }
                            UserController.updateNameReq(text);
                        }
                    }

                    Button {
                        text: "修改"
                        background: Rectangle {
                            color: Theme.backgroundColor
                            radius: 6
                            border.width: 1
                            border.color: Theme.paratextColor
                        }
                        onClicked: usernameField.accepted();
                    }
                }
            }

            // 修改头像
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
                    text: "修改头像"
                    font.pixelSize: 18
                    font.bold: true
                    color: Theme.highlightingColor
                }

                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignLeft
                    // 头像
                    Image {
                        id: avatarImage
                        Layout.preferredHeight: 100
                        Layout.preferredWidth: 100
                        Layout.alignment: Qt.AlignCenter
                        fillMode: Image.PreserveAspectFit
                        source: "qrc:/icons/user.svg"
                        // 图片圆角
                        layer.enabled: true
                        layer.smooth: true
                        layer.effect: OpacityMask {
                            maskSource: Rectangle {
                                width: avatarImage.width
                                height: avatarImage.height
                                radius: avatarImage.width / 2
                            }
                        }
                        cache: true
                        asynchronous: true
                        mipmap: true 
                        // 信号
                        Connections {
                            target: UserController
                            function onLoginChanged() {
                                // 登录状态变化时, 刷新头像
                                if (UserController.isLoggedIn()) {
                                    // 已登录, 刷新头像
                                    avatarImage.source = "image://netImagePoll/user/avatar/get?" + Date.now();
                                } else {
                                    // 未登录, 使用默认头像
                                    avatarImage.source = "qrc:/icons/user.svg";
                                }
                            }
                        }
                    }
                    DropArea {
                        id: dropArea
                        width: 100
                        height: 100
                        Rectangle {
                            anchors.fill: parent
                            color: Theme.backgroundColor
                            radius: 8
                        }
                        Text {
                            anchors.centerIn: parent
                            text: "拖入图片上传"
                            color: Theme.paratextColor
                            visible: !dropArea.containsDrag;
                        }
                        onDropped: (drop) => {
                            if (drop.hasUrls) {
                                const url = drop.urls[0];
                                const path = decodeURIComponent(url).replace("file://", "");
                                UserController.updateAvatarReq(path);
                            }
                        }
                    }

                    // 个性签名 @todo
                }
            }

            // 修改密码
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
                    text: "修改密码"
                    font.pixelSize: 18
                    font.bold: true
                    color: Theme.highlightingColor
                }

                IconTextField {
                    id: oldPasswordField
                    Layout.alignment: Qt.AlignLeft
                    Layout.preferredWidth: 230
                    iconDefaultColor: Theme.textColor
                    iconHighlightColor: Theme.highlightingColor
                    borderHighlightColor: Theme.highlightingColor
                    iconSource: "qrc:/icons/lock.svg"
                    placeholderText: "旧密码"
                    isPasswordMode: true
                    clickableIconSource: "qrc:/icons/unlock.svg"
                    onIconClicked: (isDefault) => {
                        isPasswordMode = isDefault;
                    }
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
                    placeholderText: "新密码"
                    isPasswordMode: true
                    clickableIconSource: "qrc:/icons/unlock.svg"
                    onIconClicked: (isDefault) => {
                        isPasswordMode = isDefault;
                    }
                    KeyNavigation.tab: password2Field
                    onAccepted: password2Field.accepted()
                }
                RowLayout {
                    spacing: 8
                    Layout.alignment: Qt.AlignLeft

                    IconTextField {
                        id: password2Field
                        Layout.alignment: Qt.AlignLeft
                        Layout.preferredWidth: 230
                        iconDefaultColor: Theme.textColor
                        iconHighlightColor: Theme.highlightingColor
                        borderHighlightColor: Theme.highlightingColor
                        iconSource: "qrc:/icons/lock.svg"
                        placeholderText: "再次输入新密码"
                        isPasswordMode: true
                        clickableIconSource: "qrc:/icons/unlock.svg"
                        onIconClicked: (isDefault) => {
                            isPasswordMode = isDefault;
                        }
                        onAccepted: {
                            if (oldPasswordField.text.length === 0) {
                                MessageController.showError("旧密码不能为空");
                                return;
                            }
                            if (password1Field.text.length === 0) {
                                MessageController.showError("新密码不能为空");
                                return;
                            }
                            if (password1Field.text !== password2Field.text) {
                                MessageController.showError("两次输入的新密码不一致");
                                return;
                            }
                            UserController.updatePassword(oldPasswordField.text, password1Field.text);
                        }
                    }

                    Button {
                        text: "修改"
                        background: Rectangle {
                            color: Theme.backgroundColor
                            radius: 6
                            border.width: 1
                            border.color: Theme.paratextColor
                        }
                        onClicked: password2Field.accepted()
                    }
                }
            }
        }
    }
}