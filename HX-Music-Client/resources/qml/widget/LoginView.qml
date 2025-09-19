pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "./internal"
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
            Layout.preferredHeight: 150
            Layout.preferredWidth: 150
            Layout.alignment: Qt.AlignCenter
            fillMode: Image.PreserveAspectFit
            source: `image://netImagePoll/user/avatar/get`
        }

        // 后端网址
        IconTextField {
            id: backendUrlField
            Layout.preferredWidth: 500
            Layout.alignment: Qt.AlignCenter
            iconDefaultColor: Theme.textColor
            iconHighlightColor: Theme.highlightingColor
            borderHighlightColor: Theme.highlightingColor
            iconSource: "qrc://icons/cloud-server.svg"
            placeholderText: "https://..."
            text: UserConfig.getBackendUrl()
            onTextChanged: {
                UserConfig.setBackendUrl(text);
            }
            // 连接 UserConfig 的 backendUrl 属性变化
            Connections {
                target: UserConfig
                onBackendUrlChanged: {
                    const url = UserConfig.getBackendUrl();
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
                    iconSource: "qrc://icons/user.svg"
                    placeholderText: "用户名"
                    text: UserConfig.getName()
                    onTextChanged: {
                        UserConfig.setName(text);
                    }
                    Connections {
                        target: UserConfig
                        onNameChanged: {
                            const name = UserConfig.getName();
                            if (name !== usernameField.text) {
                                usernameField.text = name;
                            }
                        }
                    }
                }
                GlowButton {
                    // 不要 tab 键聚焦
                    focusPolicy: Qt.NoFocus
                    Layout.alignment: Qt.AlignRight
                    text: "Logout"
                    baseColor: Theme.backgroundColor
                    glowColor: Theme.highlightingColor
                    accentColor: Theme.highlightingColor
                    enabled: false
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
                    iconSource: "qrc://icons/lock.svg"
                    placeholderText: "密码"
                    isPasswordMode: true
                }
                GlowButton {
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