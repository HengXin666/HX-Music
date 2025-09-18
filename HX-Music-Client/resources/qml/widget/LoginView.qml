pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "./internal"

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
            source: `image://netImagePoll/user/avatar/${1}` // @todo
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
            text: Theme.getBackendUrl()
            onTextChanged: {
                Theme.setBackendUrl(text);
                MessageController.showInfo("服务器地址已自动保存");
            }
            // 连接 Theme 的 backendUrl 属性变化
            Connections {
                target: Theme
                onBackendUrlChanged: {
                    const url = Theme.getBackendUrl();
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
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 230
                    iconDefaultColor: Theme.textColor
                    iconHighlightColor: Theme.highlightingColor
                    borderHighlightColor: Theme.highlightingColor
                    iconSource: "qrc://icons/user.svg"
                    placeholderText: "用户名"
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