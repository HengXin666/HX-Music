pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import "qrc:/HX/Music/qml/widget/internal"
import HX.Music

Item {
    ColumnLayout {
        anchors.fill: parent
        spacing: 15
        Item {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
        }

        // 图片
        Image {
            id: avatarImage
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
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

        // 后端网址
        IconTextField {
            id: backendUrlField
            Layout.preferredWidth: 500
            Layout.alignment: Qt.AlignCenter
            iconDefaultColor: Theme.textColor
            iconHighlightColor: Theme.highlightingColor
            borderHighlightColor: Theme.highlightingColor
            iconSource: "qrc:/icons/cloud-server.svg"
            placeholderText: "https://..."
            text: UserController.getBackendUrl()
            onTextChanged: {
                UserController.setBackendUrl(text);
            }
            // 连接 UserController 的 backendUrl 属性变化
            Connections {
                target: UserController
                function onBackendUrlChanged() {
                    const url = UserController.getBackendUrl();
                    if (url !== backendUrlField.text) {
                        backendUrlField.text = url;
                    }
                }
            }
        }

        // 用户名 - 密码
        RowLayout {
            Layout.preferredWidth: 500
            Layout.alignment: Qt.AlignCenter
            ColumnLayout {
                Layout.alignment: Qt.AlignLeft
                spacing: 15
                IconTextField {
                    id: usernameField
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 230
                    iconDefaultColor: Theme.textColor
                    iconHighlightColor: Theme.highlightingColor
                    borderHighlightColor: Theme.highlightingColor
                    iconSource: "qrc:/icons/user.svg"
                    placeholderText: "用户名"
                    text: UserController.getName()
                    onTextChanged: {
                        UserController.setName(text);
                    }
                    onAccepted: loginButton.clicked()
                    KeyNavigation.tab: passwordField
                    Connections {
                        target: UserController
                        function onNameChanged() {
                            const name = UserController.getName();
                            if (name !== usernameField.text) {
                                usernameField.text = name;
                            }
                        }
                    }
                }
                GlowButton {
                    // 不要 tab 键聚焦
                    id: logoutButton
                    focusPolicy: Qt.NoFocus
                    Layout.alignment: Qt.AlignRight
                    text: "Logout"
                    baseColor: Theme.backgroundColor
                    glowColor: Theme.highlightingColor
                    accentColor: Theme.highlightingColor
                    enabled: UserController.isLoggedIn()
                    onClicked: {
                        UserController.logoutReq();
                        MessageController.showWarning("已退出登录");
                    }
                    KeyNavigation.tab: loginButton
                    // 信号
                    Connections {
                        target: UserController
                        function onLoginChanged() {
                            logoutButton.enabled = UserController.isLoggedIn();
                        }
                    }
                }
            }
            ColumnLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 15
                IconTextField {
                    id: passwordField
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
                    onAccepted: loginButton.clicked()
                }
                GlowButton {
                    id: loginButton
                    Layout.alignment: Qt.AlignLeft
                    text: "Login"
                    baseColor: Theme.backgroundColor
                    glowColor: Theme.highlightingColor
                    accentColor: Theme.highlightingColor
                    onClicked: {
                        // 登录
                        UserController.loginReq(usernameField.text, passwordField.text);
                    }
                }
            }
        }

        // 是否已登录 ---

        Item {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignBottom
        }
    }
}