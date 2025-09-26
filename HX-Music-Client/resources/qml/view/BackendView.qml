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
                    Button {
                        text: "开始扫描"
                        background: Rectangle {
                            color: Theme.backgroundColor
                            radius: 6
                            border.width: 1
                            border.color: Theme.highlightingColor
                        }
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
                    Button {
                        text: "开始扫描"
                        background: Rectangle {
                            color: Theme.backgroundColor
                            radius: 6
                            border.width: 1
                            border.color: Theme.highlightingColor
                        }
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
                Button {
                    text: "清空日志"
                    Layout.alignment: Qt.AlignRight
                    background: Rectangle {
                        color: Theme.backgroundColor
                        radius: 6
                        border.width: 1
                        border.color: Theme.highlightingColor
                    }
                    onClicked: {
                        logArea.text = "";
                    }
                }
            }
        }
    }
}