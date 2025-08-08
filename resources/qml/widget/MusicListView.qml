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

    MusicListModel {
        id: musicListModel
    }

    // 拖拽文件
    DropArea {
        anchors.fill: parent
        onDropped: drop => {
            for (const urlStr of drop.urls) {
                let path = decodeURIComponent(urlStr).replace("file://", "");
                musicListModel.addFromPath(path);
            }
        }
    }

    property real maxTitleColumnWidth: 0
    property real maxAlbumColumnWidth: 0

    ListView {
        id: listView
        model: musicListModel
        anchors.fill: parent
        clip: true
        delegate: Item {
            id: delegateRoot
            required property int index
            required property var model
            width: listView.width
            height: 60

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    console.log("播放:", delegateRoot.index);
                    // MusicPlayer.play(delegateRoot.index) // 调用播放
                }
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 5
                spacing: 10

                // 序号
                Text {
                    text: (delegateRoot.index + 1).toString()
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    Layout.preferredWidth: 40
                    Layout.alignment: Qt.AlignVCenter
                }

                // 封面 + 标题 + 歌手(靠左, 自适应宽度)
                RowLayout {
                    id: titleColumn
                    Layout.alignment: Qt.AlignLeft
                    spacing: 8
                    Layout.preferredWidth: root.maxTitleColumnWidth
                    onWidthChanged: {
                        if (width > root.maxTitleColumnWidth) {
                            root.maxTitleColumnWidth = width;
                        }
                    }

                    // 封面
                    Rectangle {
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        color: "#0a7d92"
                        Image {
                            anchors.fill: parent
                            source: `image://imgPool/${delegateRoot.model.url}`
                            fillMode: Image.PreserveAspectCrop
                        }
                    }

                    // 标题 + 歌手
                    ColumnLayout {
                        spacing: 2
                        Layout.alignment: Qt.AlignLeft
                        Layout.fillWidth: true

                        Text {
                            text: delegateRoot.model.title
                            font.pixelSize: 16
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }

                        RowLayout {
                            spacing: 5
                            Layout.fillWidth: true

                            Repeater {
                                model: delegateRoot.model.artist
                                Text {
                                    required property var modelData
                                    text: modelData
                                    font.pixelSize: 14
                                    color: "#e62727"
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }

                // 专辑(靠左, 自适应宽度)
                Text {
                    id: albumColumn
                    text: delegateRoot.model.album.length > 16
                            ? delegateRoot.model.album.substring(0, 16) + "…"
                            : delegateRoot.model.album
                    font.pixelSize: 14
                    color: "#079f25"
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: root.maxAlbumColumnWidth
                    onWidthChanged: {
                        if (width > root.maxAlbumColumnWidth) {
                            root.maxAlbumColumnWidth = width;
                        }
                    }
                }

                // 时长(固定宽度, 永远贴最右)
                Text {
                    text: root.formatDuration(delegateRoot.model.duration)
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignRight
                    Layout.preferredWidth: 60
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
            }
        }
    }

    // 时间格式化
    function formatDuration(seconds) {
        let min = Math.floor(seconds / 60);
        let sec = seconds % 60;
        return (min < 10 ? "0" : "") + min + ":" + (sec < 10 ? "0" : "") + sec;
    }
}
