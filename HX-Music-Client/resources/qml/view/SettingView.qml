import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import HX.Music
import "qrc:/HX/Music/qml/widget/internal"

Item {
    id: root
    width: 500
    height: 400

    property alias serverUrl: serverInput.text
    property alias autoCenterLyrics: autoCenterSwitch.checked

    ScrollView {
        id: scrollArea
        anchors.fill: parent
        clip: true

        // 解决 rightPadding 循环绑定问题, 不直接依赖 ScrollBar.width
        rightPadding: 12

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AlwaysOff
            interactive: true
            // 使用 implicitWidth 避免和 ScrollView 相互绑定
            implicitWidth: 12
        }

        ColumnLayout {
            id: contentLayout
            width: parent.width
            spacing: 20
            anchors.margins: 20

            // 标题
            Text {
                text: "设置"
                font.pixelSize: 24
                color: Theme.highlightingColor
                Layout.alignment: Qt.AlignCenter
            }

            // 后端服务器设置
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
                        text: "后端服务器"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                Text {
                    text: "服务器地址"
                    color: Theme.textColor
                    font.pixelSize: 16
                }
                // 后端网址
                IconTextField {
                    id: serverInput
                    Layout.preferredWidth: 500
                    Layout.alignment: Qt.AlignCenter
                    iconDefaultColor: Theme.textColor
                    iconHighlightColor: Theme.highlightingColor
                    borderHighlightColor: Theme.highlightingColor
                    iconSource: "qrc:/icons/cloud-server.svg"
                    placeholderText: "https://..."
                    text: UserController.getBackendUrl()
                    onTextChanged: {
                        UserController.setBackendUrl(serverInput.text);
                        MessageController.showInfo("服务器地址已自动保存");
                    }
                    // 连接 UserController 的 backendUrl 属性变化
                    Connections {
                        target: UserController
                        function onBackendUrlChanged() {
                            const url = UserController.getBackendUrl();
                            if (url !== serverInput.text) {
                                serverInput.text = url;
                            }
                        }
                    }
                }
            }

            // 主题颜色设置
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Theme.paratextColor
                        opacity: 0.3
                    }

                    Text {
                        text: "主题颜色"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                RowLayout {
                    spacing: 10
                    ColorPicker {
                        id: textPicker
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        selectedColor: Theme.textColor
                        onAccepted: {
                            Theme.textColor = selectedColor;
                            MessageController.showInfo("文本颜色已自动保存");
                        }
                        Connections {
                            target: Theme
                            function onTextColorChanged() {
                                textPicker.selectedColor = Theme.textColor;
                            }
                        }
                    }
                    Label {
                        text: "文本颜色"
                        color: Theme.textColor
                    }
                }

                RowLayout {
                    spacing: 10
                    ColorPicker {
                        id: paratextPicker
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        selectedColor: Theme.paratextColor
                        onAccepted: {
                            Theme.paratextColor = selectedColor;
                            MessageController.showInfo("副文本颜色已自动保存");
                        }
                        Connections {
                            target: Theme
                            function onParatextColorChanged() {
                                paratextPicker.selectedColor = Theme.paratextColor;
                            }
                        }
                    }
                    Label {
                        text: "副文本颜色"
                        color: Theme.paratextColor
                    }
                }

                RowLayout {
                    spacing: 10
                    ColorPicker {
                        id: highlightingPicker
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        selectedColor: Theme.highlightingColor
                        onAccepted: {
                            Theme.highlightingColor = selectedColor;
                            MessageController.showInfo("高亮颜色已自动保存");
                        }
                        Connections {
                            target: Theme
                            function onHighlightingColorChanged() {
                                highlightingPicker.selectedColor = Theme.highlightingColor;
                            }
                        }
                    }
                    Label {
                        text: "高亮颜色"
                        color: Theme.highlightingColor
                    }
                }

                RowLayout {
                    spacing: 10
                    ColorPicker {
                        id: backgroundPicker
                        Layout.preferredWidth: 26
                        Layout.preferredHeight: 26
                        selectedColor: Theme.backgroundColor
                        onAccepted: {
                            Theme.backgroundColor = selectedColor;
                            MessageController.showInfo("背景颜色已自动保存");
                        }
                        Connections {
                            target: Theme
                            function onBackgroundColorChanged() {
                                backgroundPicker.selectedColor = Theme.backgroundColor;
                            }
                        }
                    }
                    Label {
                        text: "背景颜色"
                        color: Theme.backgroundColor
                    }
                }

                TextButton {
                    text: "重置颜色主题"
                    onClicked: {
                        Theme.textColor = "#ffffff";
                        Theme.paratextColor = "#b3b3b3";
                        Theme.highlightingColor = "#ff13ff";
                        Theme.backgroundColor = "#121212";
                        MessageController.showWarning("重置颜色主题已重置!");
                    }
                }

                RowLayout {
                    spacing: 10

                    // 显示当前选择的背景图预览
                    Rectangle {
                        width: 60
                        height: 40
                        radius: 4
                        border.color: Theme.paratextColor
                        border.width: 1
                        color: Theme.backgroundColor
                        clip: true

                        Image {
                            anchors.fill: parent
                            source: Theme.backgroundImgUrl
                            fillMode: Image.PreserveAspectCrop
                        }
                    }

                    TextButton {
                        text: "选择背景图片"
                        onClicked: fileDialog.open()
                    }

                    // 文件选择对话框
                    FileDialog {
                        id: fileDialog
                        title: "选择背景图片"
                        nameFilters: ["图片文件 (*.png *.jpg *.jpeg)"]
                        onAccepted: {
                            if (selectedFile !== "") {
                                Theme.backgroundImgUrl = selectedFile;
                                MessageController.showInfo("背景图片已自动保存");
                            }
                        }
                    }

                    TextButton {
                        text: "清除背景"
                        onClicked: {
                            Theme.backgroundImgUrl = Theme.defaultBackgroundImgUrl;
                            MessageController.showInfo("背景图片已清除并自动保存");
                        }
                    }
                }
            }

            // 歌词设置
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

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
                        text: "歌词设置"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.highlightingColor
                    }
                }

                RowLayout {
                    spacing: 10
                    Layout.margins: 10

                    Label {
                        text: "自动居中歌词"
                        color: Theme.textColor
                        font.pixelSize: 16
                    }

                    Switch {
                        id: autoCenterSwitch
                        checked: LyricController.isAutoCenter
                        onCheckedChanged: {
                            LyricController.isAutoCenter = checked;
                            LyricController.renderAFrameInstantly();
                            MessageController.showInfo("自动居中歌词: " + checked);
                        }
                    }
                }
            }
        }
    }
}
